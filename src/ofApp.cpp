#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
#ifdef __APPLE__
    r.setup(57200);
#else
    r.setup(7000);
#endif
	ofBackground(0, 0, 0);
	test.listDevices();

	test.setDeviceID(2);
	test.setup(1920, 1080);

	box2d.init();
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


    
    chainevent.addEvent(3., BEGIN_LEARNING);
    chainevent.addEvent(3., LEARNING);
    chainevent.addEvent(20., TRAINING, true);
    chainevent.addEvent(2., PLAYING, true);
    chainevent.addEvent(3., HIT1);
    chainevent.addEvent(3., HIT2);
    chainevent.addEvent(5., POUR);
    chainevent.addEvent(3., RESET);
    chainevent.addEvent(3., TRYAGAIN);
    

    learnedPoses.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
    learnedPoses.begin();
    ofClear(0);
    learnedPoses.end();

	userFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA32F_ARB);
	userFbo.begin();
	ofClear(0);
	userFbo.end();

	feedBackFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
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

				theUsers[id].pointsInView = 18;
				numHumans = MAX(id + 1, numHumans);
				
				
				int indx = 0;
				for (int i = 0; i < m.getNumArgs(); i += 3) {
					if (m.getArgType(i) == 102) {
                        float x = m.getArgAsFloat(i) / SCALE_X;
                        float y = m.getArgAsFloat(i + 1) / SCALE_Y;
						// cout << x<< " "  << y << endl;
                        if(x > left && x < right+left && y > top && y < bottom+top){
                            theUsers[id].pointsInView --;
							// cout << "true" << endl;
                        }
                        
						theUsers[id].addPoint(indx, x, y);
						indx++;
					}
				}
			}
		}
    }

    chainevent.update();
    
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
    ofDrawRectangle(left* ofGetWidth(), top*ofGetHeight(), (right)*ofGetWidth(), (bottom)*ofGetHeight());
    if(drinkSequence[currentDrinkSequence]<poseImages.size()){
        
        for(int i = 0; i<averagePoses[currentDrinkSequence].size(); i+=2) {
            ofDrawCircle(averagePoses[currentDrinkSequence][i] * SCALE_X, averagePoses[currentDrinkSequence][i+1] * SCALE_Y, 20);
        }
        
        poseImages[drinkSequence[currentDrinkSequence]].draw(0,0, ofGetWidth(), ofGetHeight());
    }
	feedBackFbo.end();

	int messageX = 50;
	int messageY = ofGetHeight() - 70;

	if (numHumansInView == 1) {
        vector<double>sample;
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
                    
                    vector<double>newAverage;
                    newAverage = sample;
                    averagePoses.push_back(newAverage);
                    
                    chainevent.next();
                }

                break;
            }
            case LEARNING: {
                
                classifier.addSample(sample, numPoses);
                std::transform (averagePoses[numPoses].begin(), averagePoses[numPoses].end(), sample.begin(), averagePoses[numPoses].begin(), std::plus<double>());
                numSamples ++;
                // foo: 21 41 61 81 101
                learnedPoses.begin();
				ofFill();
				//ofSetLineWidth(10);
                ofSetColor(0, 0, 255);
                u.draw();
				ofNoFill();
                learnedPoses.end();
 
                
                if (chainevent.getTime()>chainevent.getDuration() - 0.1) {
                    ofPixels pix;
                    learnedPoses.readToPixels(pix);
                    ofSaveImage(pix, "outputs\\" + ofToString(numPoses)+".png");
                    
                    ofImage img;
                    img.load("outputs\\" + ofToString(numPoses)+".png");
                    poseImages.push_back(img);
                    
                    std::transform(averagePoses[numPoses].begin(), averagePoses[numPoses].end(), averagePoses[numPoses].begin(),
                                   std::bind1st(std::multiplies<double>(),1.0/double(numSamples+1)));
                    
                    numSamples = 0;
                    numPoses++;
                    if (numPoses < 3)chainevent.beginEvents();
                    else {
                        classifier.beginTraining();
                        chainevent.next();
                    }
                }
                break;
            }
                
                
            case TRAINING: {
                //statements
				feedBackFbo.begin();
				font.drawString("Just a second.. Im learning", messageX, messageY);
				feedBackFbo.end();
                if (classifier.isTrained())chainevent.next();
                break;
            }
            case PLAYING: {
                
                //statements
                int currentPose = classifier.predict(sample);

                if (currentPose == drinkSequence[currentDrinkSequence]) {
                    chainevent.next();
                    currentDrinkSequence ++;
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

                int currentPose = classifier.predict(sample);
                if (currentPose == drinkSequence[currentDrinkSequence]) {
                    chainevent.next();
                    currentDrinkSequence ++;
                }
                if (chainevent.getDuration() - chainevent.getTime() < 0.1){
                    chainevent.setTo(TRYAGAIN);
                }
                
                break;
            }
            case HIT2: {
                feedBackFbo.begin();
                font.drawString("Now, do the last one to get a drinks!!", messageX, messageY);
                float chainEventTime = (chainevent.getDuration()-chainevent.getTime())/chainevent.getDuration();
                ofDrawRectangle(0, ofGetHeight() - 50, chainEventTime * ofGetWidth(), 50);
                feedBackFbo.end();
                
                int currentPose = classifier.predict(sample);
                if (currentPose == drinkSequence[currentDrinkSequence]) {
                    chainevent.next();
                }
                if (chainevent.getDuration() - chainevent.getTime() < 0.1){
                    chainevent.setTo(TRYAGAIN);
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
        drawUserWithPngs(&theUsers[i]);
	}
	userFbo.end();

	box2d.update();
	//test.update();
}
void ofApp::reset() {
    classifier.clearTrainingInstances();
}
void ofApp::drawUserWithPngs(user *u){
    /*
    ofVec2f p1, p2, p3;
    ofImage img;

    vector<int>numParts = {4,3,3,2,5,6,6,7,8,9,9,10,11,12,12,13};
    vector<string>names = {"Left-Arm-Outer.png", "Left-Arm-Inner.png", "Right-Arm-Inner.png", "Right-Arm-Outer.png", "Left-Leg-Upper.png", "Left-Leg-Lower.png", "Right-Leg-Upper.png", "Right-Leg-Lower.png"};
    for(int i = 0; i< numParts.size(); i+=2){
        p1 = u->circles[numParts[i]]->getPosition();
        p2 = u->circles[numParts[i+1]]->getPosition();
        p3 = getMean(p1, p2);
        
        img = bodyPartImages[names[i/2]];
        ofPushMatrix();
        ofTranslate(p3.x, p3.y);
        ofRotate(getAngle(p1, p2));
		img.draw(-(img.getWidth()*scale) / 2, -(img.getHeight()*scale) / 2, img.getWidth()*scale, img.getHeight()*scale);
        ofPopMatrix();
    }
    
    p1 = u->circles[2]->getPosition();
    p2 = u->circles[5]->getPosition();
    ofVec2f p4 = u->circles[8]->getPosition();
    ofVec2f p5 = u->circles[11]->getPosition();
    p3 = (p1+p2+p4+p5)/4;
    ofPushMatrix();
    ofTranslate(p3.x, p3.y);
    ofRotate(getAngle(u->circles[1]->getPosition(), u->circles[14]->getPosition()));
    float w = (u->circles[5]->getPosition() - u->circles[2]->getPosition()).length();
    float h = (u->circles[1]->getPosition() - getMean(p4, p5)).length();
	//cout << w << " " << h << endl;

    img = bodyPartImages["Torso.png"];
    img.draw(-w/2, -h/2, w, h);
    ofPopMatrix();
             
    p1 = u->circles[0]->getPosition();
    p1 = u->circles[1]->getPosition();
    img = bodyPartImages["Head.png"];
    
    ofPushMatrix();
    ofTranslate(p1.x, p1.y);
    ofRotate(getAngle(p1, p2));
    img.draw(-(img.getWidth()*scale)/2, -(img.getHeight()*scale), img.getWidth()*scale, img.getHeight()*scale);
    ofPopMatrix();
    
    */
	vector<vector<int>> indx = { { 4,3,0 },{ 3,2,0 },{ 5,6,0 },{ 6,7,0 },{ 8,9,-90 },{ 9,10,-90 },{ 11,12,-90 },{ 12,13,-90 } };

	for (int i = 0; i < indx.size(); i++) {
		float rotation = getAngle(u->circles[indx[i][0]]->getPosition(), u->circles[indx[i][1]]->getPosition());
		ofVec2f position = getMean(u->circles[indx[i][0]]->getPosition(), u->circles[indx[i][1]]->getPosition());
		float w = bodyPartImages[i].getWidth();
		float h = (u->circles[indx[i][1]]->getPosition() - u->circles[indx[i][0]]->getPosition()).length();
		ofPushMatrix();
		ofTranslate(position.x, position.y);
		ofRotate(rotation - 90);
		bodyPartImages[i].draw(-(w*scale) / 2, -(h)/2, w*scale, h);
		ofPopMatrix();
	}
	ofVec2f p1 = u->circles[2]->getPosition();
	ofVec2f p2 = u->circles[5]->getPosition();
	ofVec2f p4 = u->circles[8]->getPosition();
	ofVec2f p5 = u->circles[11]->getPosition();
	ofVec2f p3 = (p1 + p2 + p4 + p5) / 4;
	ofPushMatrix();
	ofTranslate(p3.x, p3.y);
	ofRotate(getAngle(u->circles[1]->getPosition(), u->circles[14]->getPosition())-90);
	float w = (u->circles[5]->getPosition() - u->circles[2]->getPosition()).length() * 1.2;
	float h = (u->circles[1]->getPosition() - getMean(p4, p5)).length() * 1.1;
	int img = 8;
	if (w / h < bellyThreshold)img = 1;
	w = MAX(30, w);
	//cout << w << " " << h << endl;

	bodyPartImages[img].draw(-w / 2, -h / 2, w, h);
	ofPopMatrix();

	p1 = u->circles[0]->getPosition();
	p2 = u->circles[1]->getPosition();

	ofPushMatrix();
	ofTranslate(getMean(p1, p2).x, getMean(p1, p2).y);
	ofRotate(getAngle(p1, p2)-90);
	bodyPartImages[9].draw(-(bodyPartImages[9].getWidth()*scale) / 2, -(bodyPartImages[9].getHeight()*scale)/2, bodyPartImages[9].getWidth()*scale, bodyPartImages[9].getHeight()*scale);
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
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if (key == 'l')chainevent.beginEvents();
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
