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
	box2d.setGravity(0, 0.3);
	box2d.setFPS(60.0);

	box2d.disableEvents();
	box2d.disableGrabbing();

	theUsers.resize(MAX_USERS);
	for (int i = 0; i < MAX_USERS; i++) {
		theUsers[i].setup(&box2d);
	}
	ofEnableAlphaBlending();


    
    chainevent.addEvent(10., BEGIN_LEARNING);
    chainevent.addEvent(3., LEARNING);
    chainevent.addEvent(20., TRAINING, true);
    chainevent.addEvent(2., PLAYING, true);
    chainevent.addEvent(3., HIT1);
    chainevent.addEvent(3., HIT2);
    chainevent.addEvent(5., POUR);
    chainevent.addEvent(3., RESET);
    chainevent.addEvent(3., TRYAGAIN);
    
    parts = {"Nose", "Neck", "Right_Shoulder", "Right_Elbow", "Right_Wrist", "Left_Shoulder", "Left_Elbow", "Left_Wrist", "Right_Hip", "Right_Knee", "Right_Ankle", "Left_Hip", "Left_Knee", "Left_Ankle", "Right_Eye", "Left_Eye", "Right_Ear", "Left_Ear", "Background"};
    
    learnedPoses.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
    learnedPoses.begin();
    ofClear(0);
    learnedPoses.end();

	userFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	userFbo.begin();
	ofClear(0);
	userFbo.end();

	feedBackFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	feedBackFbo.begin();
	ofClear(0);
	feedBackFbo.end();

	font.load("Brandon_med.otf", 32);
    
    gui.setup();
    gui.add(left.set("left", 0, 0, ofGetWidth()));
    gui.add(right.set("right", 0, 0, ofGetWidth()));
    gui.add(top.set("top", 0, 0, ofGetHeight()));
    gui.add(bottom.set("bottom", 0, 0, ofGetHeight()));
    
    gui.loadFromFile("settings.xml");
    

    drinkSequence.clear();
    drinkSequence = {1, 2, 3};

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
#ifdef __APPLE__
    // not up to date!
    while (r.hasWaitingMessages()) {
        ofxOscMessage m;
        r.getNextMessage(m);
        
        string result = m.getArgAsString(0);
        ofxJSONElement json = result;
        const Json::Value& bodyPoints = json["results"];
        
        for(int i = 0; i<MIN(MAX_USERS, bodyPoints["humans"].size()); i++){
            int partIndx = 0;
            
            theUsers[i].clearPoints();
            
            for(int u = 0; u<bodyPoints["humans"][i].size(); u++){
                isFrameNew = true;
                string part = bodyPoints["humans"][i][u][0].asString();
                double x = bodyPoints["humans"][i][u][1].asDouble();
                double y = bodyPoints["humans"][i][u][2].asDouble();
                
                for(int p = partIndx; p<parts.size(); p++){
                    if(parts[p]==part){
                        theUsers[i].addPoint(p , x ,y);
                        partIndx = p;
                        break;
                    }
                }
            }
        }
    }
#else
    while (r.hasWaitingMessages()) {
        ofxOscMessage m;
        r.getNextMessage(m);
        
        // cout << m.getAddress() << endl;
		if (m.getAddress() == "/nohumans")numHumans = 0;
		else
		{
			int id = ofToInt(ofSplitString(m.getAddress(), "person").back());
            theUsers[id].pointsInView = 0;
            
            numHumans = MAX(id, numHumans);
            
			if (id<theUsers.size()) {
				isFrameNew = true;
				// cout << m.getNumArgs() << endl;
				int indx = 0;
				for (int i = 0; i < m.getNumArgs(); i += 3) {
					if (m.getArgType(i) == 102) {
                        float x = m.getArgAsFloat(i);
                        float y = m.getArgAsFloat(i + 1);
                        
                        if(x - left < right && y - top < bottom){
                            theUsers[id].pointsInView ++;
                        }
                        
						theUsers[id].addPoint(indx, x, y);
						indx++;
					}
				}
			}
		}
    }
#endif
    chainevent.update();
    
    int userInView = -1;
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
    ofDrawRectangle(left, top, right-left, bottom-top);
    if(drinkSequence[currentDrinkSequence]<poseImages.size()){
        
        for(int i = 0; i<averagePoses[currentDrinkSequence].size(); i+=2) {
            ofDrawCircle(averagePoses[currentDrinkSequence][i] * SCALE_X, averagePoses[currentDrinkSequence][i+1] * SCALE_Y, 20);
        }
        
        poseImages[drinkSequence[currentDrinkSequence]].draw(0,0);
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
                ofSetColor(255, 255, 255, 100);
                ofNoFill();
                u.draw();
                learnedPoses.end();
 
                
                if (chainevent.getTime()>chainevent.getDuration() - 0.1) {
                    ofPixels pix;
                    learnedPoses.readToPixels(pix);
                    ofSaveImage(pix, "outputs\\" + ofToString(numPoses)+".png");
                    
                    ofImage img;
                    img.load("outputs\\" + ofToString(numPoses)+".png");
                    poseImages.push_back(img);
                    
                    std::transform(averagePoses[numPoses].begin(), averagePoses[numPoses].end(), averagePoses[numPoses].begin(),
                                   std::bind1st(std::multiplies<double>(),1.0/double(numSamples)));
                    
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
					font.drawString("Ahh.. Not really!", messageX, messageY);
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
    else if (numHumans>1) {
		feedBackFbo.begin();
		font.drawString("Please, one at a time!", messageX, messageY);
		feedBackFbo.end();
	}
	else if (numHumans == 0) {
		feedBackFbo.begin();
		font.drawString("Anoone there?", messageX, messageY);
		feedBackFbo.end();
	}
    else if (numHumans > 0 && numHumansInView == 0) {
        feedBackFbo.begin();
        font.drawString("One of you, step inside the square to try the pose machine!", messageX, messageY);
        feedBackFbo.end();
    }

	userFbo.begin();
	ofFill();
	ofSetColor(255, 255, 255, 40);
	ofDrawRectangle(0,0,userFbo.getWidth(), userFbo.getHeight());
	ofSetColor(255, 255, 255);
	ofNoFill();
	for (int i = 0; i <MIN(numHumans, theUsers.size()); i++)theUsers[i].draw();
	userFbo.end();

	box2d.update();
	test.update();
}
void ofApp::reset() {
    classifier.clearTrainingInstances();
}
//--------------------------------------------------------------
void ofApp::draw() {
    

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
