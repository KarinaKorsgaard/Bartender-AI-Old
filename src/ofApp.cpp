#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

    r.setup(7000);
    
    test.listDevices();
    test.setDeviceID(1);
    test.setup(SCALE_X, SCALE_Y);
    
	ofBackground(0, 0, 0);
	box2d.init();

	for (int i = 0; i<25; i++) {
		vector<double>newAverage;
		newAverage.resize(18 * 2);
		averagePoses.push_back(newAverage);

		ofImage img;
		if (img.load("outputs\\" + ofToString(i) + ".png"))poseImages.push_back(img);
	}
	//box2d.setGravity(0, 0.0);
	//box2d.setFPS(20.0);
	//box2d.setIterations(10, 2);

	//box2d.disableEvents();
	//box2d.disableGrabbing();

	theUsers.resize(MAX_USERS);
	for (int i = 0; i < MAX_USERS; i++) {
		theUsers[i].setup(&box2d);
	}
	ofEnableAlphaBlending();

    classifier.setup();
    
    chainevent.addEvent(3., BEGIN_LEARNING);
    chainevent.addEvent(3., LEARNING);
    chainevent.addEvent(20., TRAINING, true);
    chainevent.addEvent(2., PLAYING, true);
    chainevent.addEvent(3., HIT1);
    chainevent.addEvent(3., HIT2);
    chainevent.addEvent(5., POUR);
    chainevent.addEvent(3., RESET);
    chainevent.addEvent(3., TRYAGAIN);
    

    learnedPoses.allocate(WIDTH, HEIGHT, GL_RGBA);
    learnedPoses.begin();
    ofClear(0);
    learnedPoses.end();

	userFbo.allocate(WIDTH, HEIGHT, GL_RGBA);
	userFbo.begin();
	ofClear(0);
	userFbo.end();

	feedBackFbo.allocate(WIDTH, HEIGHT, GL_RGBA);
	feedBackFbo.begin();
	ofClear(0);
	feedBackFbo.end();

	font.load("Brandon_med.otf", 32);
    
    gui.setup();
    gui.add(left.set("left", 0, 0, 1));
    gui.add(right.set("right", 0, 0,1));
    gui.add(top.set("top", 0, 0, 1));
    gui.add(bottom.set("bottom", 0, 0, 1));
	gui.add(scale.set("scale images", 0, 0, 1));
	gui.add(bellyThreshold.set("bellyThreshold", 0, 0, 1));
    gui.add(prababilityThreshold.set("prababilityThreshold", 0, 0, 1));
	gui.add(addSamples.set("addsamples", false));
	gui.add(clearSample.set("clearSample", false));
	gui.add(train.set("train", false));
    gui.loadFromFile("settings.xml");
    

    drinkSequence.clear();
    drinkSequence = {1, 2, 3};
    
    ofDirectory dir;
    dir.allowExt("png");
    dir.listDir("Bartender");
	dir.sort();

    for (int i = 0; i<dir.size(); i++ ){
        ofImage img;
        img.load(dir.getPath(i));
        bodyPartImages.push_back(img);
        cout << dir.getName(i) << endl;
    }
	chainevent.setTo(PLAYING);
}


//--------------------------------------------------------------
void ofApp::update() {
	
    /*
     OFXOSC_TYPE_INT32            = 'i',
     OFXOSC_TYPE_INT64            = 'h',
     OFXOSC_TYPE_FLOAT            = 'f',
     OFXOSC_TYPE_DOUBLE           = 'd',
     OFXOSC_TYPE_STRING           = 's',
     OFXOSC_TYPE_SYMBOL           = 'S',
     OFXOSC_TYPE_CHAR             = 'c',
     OFXOSC_TYPE_MIDI_MESSAGE     = 'm',
     OFXOSC_TYPE_TRUE             = 'T',
     OFXOSC_TYPE_FALSE            = 'F',
     OFXOSC_TYPE_NONE             = 'N',
     OFXOSC_TYPE_TRIGGER          = 'I',
     OFXOSC_TYPE_TIMETAG          = 't',
     OFXOSC_TYPE_BLOB             = 'b',
     OFXOSC_TYPE_RGBA_COLOR       = 'r',
     */
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    isFrameNew = false;

    while (r.hasWaitingMessages()) {
        ofxOscMessage m;
        r.getNextMessage(m);
		numHumans = 0;
        // cout << m.getAddress() << endl;
		if (m.getAddress() == "/nohumans") {
			numHumans = 0;
		}
		else
		{
			int id = ofToInt(ofSplitString(m.getAddress(), "person").back());
			
            
			if (id<theUsers.size()) {

				theUsers[id].pointsInView = 0;
				numHumans = MAX(id + 1, numHumans);
				
				
				int indx = 0;
				for (int i = 0; i < m.getNumArgs(); i += 3) {
					if (m.getArgType(i) == 102) {
                        float x = 1. - (m.getArgAsFloat(i) / SCALE_X);
                        float y = m.getArgAsFloat(i + 1) / SCALE_Y;
						// cout << x<< " "  << y << endl;
                        if(x > left && x < right+left && y > top && y < bottom+top){
                            theUsers[id].pointsInView++;
							// cout << "true" << endl;
                        }
                        
						theUsers[id].addPoint(indx, x, y);
						indx++;
					}
				}
			}
		}
    }

    
    
    int userInView = -1;
	numHumansInView = 0;

    for (int i = 0; i < MIN(numHumans, theUsers.size()); i++) {
        theUsers[i].update();
        if (theUsers[i].isInView) {
            numHumansInView ++;
            userInView = i;
        }
    }
    
    
	feedBackFbo.begin();
	ofClear(0);
    ofNoFill();
    ofSetColor(200, 0, 0);
    ofDrawRectangle(left* WIDTH, top*HEIGHT, (right)*WIDTH, (bottom)*HEIGHT);
	if(drinkSequence[currentDrinkSequence]<poseImages.size())poseImages[drinkSequence[currentDrinkSequence]].draw(0, 0, WIDTH, HEIGHT);
	feedBackFbo.end();

	int messageX = 50;
	int messageY = 70;

	if (numHumansInView == 1) {
        chainevent.update();
        GRT::VectorFloat sample;
        user u = theUsers[userInView];
        sample = getSample(u);
        
        switch (chainevent.getName()) {
            case BEGIN_LEARNING: {
                
				feedBackFbo.begin();
                float chainEventTime = (chainevent.getDuration()-chainevent.getTime());
				font.drawString("Get in position in "+ ofToString(chainEventTime,0)+" seconds!", messageX, messageY);
				ofDrawRectangle(0, ofGetHeight()-50, chainEventTime/chainevent.getDuration() * ofGetWidth(), 50);
				feedBackFbo.end();

                if (chainevent.getDuration()-chainevent.getTime() < 0.1) {
					learnedPoses.begin();
					ofClear(0);
					learnedPoses.end();
                    chainevent.next();
                }

                break;
            }
            case LEARNING: {
                
                classifier.addSample(sample, numPoses);
                std::transform (averagePoses[numPoses].begin(), averagePoses[numPoses].end(), sample.begin(), averagePoses[numPoses].begin(), std::plus<double>());
                numSamples ++;
 
                
                
                if (chainevent.getTime()>chainevent.getDuration() - 0.1) {
                    
                    std::transform(averagePoses[numPoses].begin(), averagePoses[numPoses].end(), averagePoses[numPoses].begin(),
                                   std::bind1st(std::multiplies<double>(),1.0/double(numSamples+1)));
                    
					learnedPoses.begin();
					ofFill();
					ofSetColor(0, 0, 255);
					u.update();
					drawUserWithPngs(u.circlePoints);
					learnedPoses.end();

					ofPixels pix;
					learnedPoses.readToPixels(pix);
					ofSaveImage(pix, "outputs\\" + ofToString(numPoses) + ".png");
					ofImage img;
					img.load("outputs\\" + ofToString(numPoses) + ".png");
					if (numPoses>poseImages.size())
						poseImages.push_back(img);
					else poseImages[numPoses] = img;

                    numSamples = 0;
					chainevent.next();
                    
                }
                break;
            }
                
                
            case TRAINING: {
                //statements
				classifier.train();
				chainevent.next();
                break;
            }
            case PLAYING: {
				currentDrinkSequence = 0;
                //statements
                classifier.updateSample(sample);
                int currentPose = classifier.label;
                if (currentPose == drinkSequence[currentDrinkSequence] && classifier.probability > prababilityThreshold) {
                    currentDrinkSequence ++;
                    test.update();
                    ofSaveImage(test.getPixels(), "images\\session_"+ofToString(session)+"1.png");
                    chainevent.next();
                    
                }
				else {
					feedBackFbo.begin();
					font.drawString("That is pose "+ofToString(currentPose)+", not "+ofToString(drinkSequence[currentDrinkSequence]), messageX, messageY);
					feedBackFbo.end();
				}
                break;
            }
                
                
            case HIT1: {
				feedBackFbo.begin();
				font.drawString("Great! Now do the next pose!", messageX, messageY);
                float chainEventTime = (chainevent.getDuration()-chainevent.getTime())/chainevent.getDuration();
                ofDrawRectangle(0, ofGetHeight() - 50, chainEventTime * ofGetWidth(), 50);
				feedBackFbo.end();

                classifier.updateSample(sample);
                int currentPose = classifier.label;
                if (currentPose == drinkSequence[currentDrinkSequence] && classifier.probability > prababilityThreshold) {
                    test.update();
                    ofSaveImage(test.getPixels(), "images\\session_"+ofToString(session)+"2.png");
                    currentDrinkSequence ++;
                    chainevent.next();
                    
                    
                }
                if (chainevent.getDuration() - chainevent.getTime() < 0.1){
                    chainevent.setTo(TRYAGAIN);
                    session++;
                }
                
                break;
            }
            case HIT2: {
                feedBackFbo.begin();
                font.drawString("Now, do the last one to get a drink!!", messageX, messageY);
                float chainEventTime = (chainevent.getDuration()-chainevent.getTime())/chainevent.getDuration();
                ofDrawRectangle(0, ofGetHeight() - 50, chainEventTime * ofGetWidth(), 50);
                feedBackFbo.end();
                
                classifier.updateSample(sample);
                int currentPose = classifier.label;
                if (currentPose == drinkSequence[currentDrinkSequence] && classifier.probability > prababilityThreshold) {
                    session++;
                    test.update();
                    ofSaveImage(test.getPixels(), "images\\session_"+ofToString(session)+"3.png");
                    chainevent.next();
                }
                if (chainevent.getDuration() - chainevent.getTime() < 0.1){
                    chainevent.setTo(TRYAGAIN);
                    session++;
                }
                
                break;
            }
            case POUR: {
				feedBackFbo.begin();
				font.drawString("Well done! Hope you put your glass there...", messageX, messageY);
				feedBackFbo.end();
                break;
            }
            case RESET: {
                // statements
                int i = ofRandom(numPoses);
                int j = ofRandom(numPoses-1);
                int k = ofRandom(numPoses-2);
                j += j >= i;
                k += k >= std::min(i, j);
                k += k >= std::max(i, j);
                
                drinkSequence.clear();
                drinkSequence = {i, j, k};
                cout << i<<j<<k << endl;
                
                currentDrinkSequence = 0;
                
                chainevent.setTo(PLAYING);
                break;
            }
            case TRYAGAIN: {
                // statements
                feedBackFbo.begin();
                font.drawString("Ej! You need to be faster than that - Try again!", messageX, messageY);
                feedBackFbo.end();
                
                if (chainevent.getDuration() - chainevent.getTime() < 0.1)chainevent.setTo(PLAYING);
                break;
            }
            default:
                break;
        }
    }
    else if (numHumansInView>1) {
		feedBackFbo.begin();
		font.drawString("one at a time!", messageX, messageY);
		feedBackFbo.end();
	}
	else if (numHumans == 0) {
		feedBackFbo.begin();
		font.drawString("Any one there?", messageX, messageY);
		feedBackFbo.end();
	}
    else if (numHumans > 0 && numHumansInView == 0) {
        feedBackFbo.begin();
        font.drawString("Step inside the square to try the pose machine!", messageX, messageY);
        feedBackFbo.end();
    }

	userFbo.begin();
    ofClear(0);
	ofSetColor(255, 255, 255);
	ofNoFill();
	for (int i = 0; i < MIN(numHumans, theUsers.size()); i++) {
		//theUsers[i].draw();
        drawUserWithPngs(theUsers[i].circlePoints);
	}
	userFbo.end();

	box2d.update();
    
    if(addSamples){
        chainevent.beginEvents();
        addSamples = false;
    }
    if(clearSample){
        classifier.clearSample(numPoses);
    }
    if(train){
		chainevent.setTo(TRAINING);
        train = false;
    }
}
void ofApp::reset() {
    //classifier.clearTrainingInstances();
}
void ofApp::drawUserWithPngs(vector<ofVec2f> p){


	ofVec2f mean = (p[2] + p[5] + p[8] + p[11]) / 4;
	ofPushMatrix();
	ofTranslate(mean.x, mean.y);
	ofRotate(getAngle(p[1], p[14]) - 90);
	float w = (p[5] - p[2]).length() * 1.2;
	float h = (p[1] - getMean(p[8], p[11])).length() * 1.1;
	int img = 13;
	if (w / h < bellyThreshold)img = 1;
	w = MAX(30, w);
	//cout << w << " " << h << endl;

	bodyPartImages[img].draw(-w / 2, -h / 2, w, h);
	ofPopMatrix();

	vector<vector<int>> indx = { { 4,3,11 },{ 3,2,0 },{ 5,6,0 },{ 7,6,12 },{ 8,9,0 },{ 10,9,9 },{ 11,12,0 },{ 13,12,10 } };

	for (int i = 0; i < indx.size(); i++) {
		float rotation = getAngle(p[indx[i][0]], p[indx[i][1]]);
		ofVec2f position = getMean(p[indx[i][0]], p[indx[i][1]]);
		float w = bodyPartImages[i].getWidth();
		float h = (p[indx[i][1]] - p[indx[i][0]]).length();
		ofPushMatrix();
		ofTranslate(position.x, position.y);
		ofRotate(rotation - 90);
		bodyPartImages[i].draw(-(w*scale) / 2, -(h)/2, w*scale, h);
		ofPopMatrix();

		if(indx[i][2]>0) {
			h = bodyPartImages[indx[i][2]].getHeight();
			ofPushMatrix();
			ofTranslate(p[indx[i][0]].x, p[indx[i][0]].y);
			ofRotate(rotation - 90);
			bodyPartImages[indx[i][2]].draw(-(w*scale) / 2, -(h*scale) / 2, w*scale, h*scale);
			ofPopMatrix();
		}
	}

	ofPushMatrix();
	ofTranslate(getMean(p[0], p[1]).x, getMean(p[0], p[1]).y);
	ofRotate(getAngle(p[0], p[1])-90);
	bodyPartImages[8].draw(-(bodyPartImages[8].getWidth()*scale) / 2, -(bodyPartImages[8].getHeight()*scale)/2, bodyPartImages[8].getWidth()*scale, bodyPartImages[8].getHeight()*scale);
	ofPopMatrix();

};


//--------------------------------------------------------------
void ofApp::draw() {
    
    ofBackgroundHex(0x22264C);
    ofSetColor(255);
	userFbo.draw(0,0);
	feedBackFbo.draw(0, 0);
    gui.draw();
	//learnedPoses.draw(0, 0, learnedPoses.getWidth() / 4, learnedPoses.getHeight() / 4);
	//test.draw(learnedPoses.getWidth() / 4, 0, learnedPoses.getWidth() / 4, learnedPoses.getHeight() / 4);
    
    if(debug){
        classifier.drawAllLabels();
        ofNoFill();
        ofSetLineWidth(5);
        ofPushMatrix();
        ofTranslate((ofGetWidth()/5)*(numPoses%5), (ofGetHeight()/5)*int(numPoses/5));
        ofDrawRectangle(0, 0, ofGetWidth()/5, ofGetHeight()/5);
        ofPopMatrix();
        ofSetLineWidth(1);
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if (key == 'l')chainevent.beginEvents();
	if (key == 'd')debug = !debug;
	if (key == 's')classifier.save();
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
    numPoses =(x/(ofGetWidth()/5)) + y/(ofGetHeight()/5) * 5;
    cout <<"x: "<<x/(ofGetWidth()/5)<<" y: "<<y/(ofGetHeight()/5) << " pose number: "<< numPoses << endl;
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {
    
}
void ofApp::exit(){
    for(int i = 0; i<theUsers.size(); i++)theUsers[i].exit();
}
