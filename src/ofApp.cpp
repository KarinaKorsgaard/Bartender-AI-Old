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

    
    poseMap.resize(NUM_DRINKS);
   // fakeUsers.resize(NUM_DRINKS);
    
    chainevent.addEvent(10., BEGIN_LEARNING);
    chainevent.addEvent(3., LEARNING);
    chainevent.addEvent(20., TRAINING, true);
    chainevent.addEvent(2., PLAYING, true);
    chainevent.addEvent(500., HIT, true);
    chainevent.addEvent(5., POUR);
    chainevent.addEvent(3., RESET);
    
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
    while (r.hasWaitingMessages()) {
        ofxOscMessage m;
        r.getNextMessage(m);
        
        string result = m.getArgAsString(0);
        ofxJSONElement json = result;
        const Json::Value& bodyPoints = json["results"];
        
        for(int i = 0; i<MIN(MAX_USERS, bodyPoints["humans"].size()); i++){
            int partIndx = 0;
            
            theUser.clearPoints();
            
            for(int u = 0; u<bodyPoints["humans"][i].size(); u++){
                isFrameNew = true;
                string part = bodyPoints["humans"][i][u][0].asString();
                double x = bodyPoints["humans"][i][u][1].asDouble();
                double y = bodyPoints["humans"][i][u][2].asDouble();
                
                for(int p = partIndx; p<parts.size(); p++){
                    if(parts[p]==part){
                        theUser.addPoint(p , x ,y);
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
		if (m.getAddress() == "/nohumans")numHumnas = 0;
		else
		{
			int id = ofToInt(ofSplitString(m.getAddress(), "person").back());
			numHumnas = id + 1;

			if (id<theUsers.size()) {
				isFrameNew = true;
				// cout << m.getNumArgs() << endl;
				int indx = 0;
				for (int i = 0; i < m.getNumArgs(); i += 3) {
					if (m.getArgType(i) == 102) {
						theUsers[id].addPoint(indx, m.getArgAsFloat(i), m.getArgAsFloat(i + 1));
						indx++;
					}
				}
			}
		}
    }
#endif
    chainevent.update();
    
    vector<double>sample;
    sample = getSample();
    
	for (int i = 0; i < MIN(numHumnas, theUsers.size()); i++)theUsers[i].update();

	feedBackFbo.begin();
	ofClear(0);
	feedBackFbo.end();

	int messageX = 50;
	int messageY = ofGetHeight() - 70;

	if (numHumnas == 1) {

        switch (chainevent.getName()) {
            case BEGIN_LEARNING: {
                
                if (chainevent.getTime() > 5.0) {
                    int drinkBeingLearned = numPoses%NUM_DRINKS;
                   // fakeUsers[drinkBeingLearned].clear();
					ofPixels pix;
					learnedPoses.readToPixels(pix);
					ofSaveImage(pix, "outputs\\" + ofToString(numPoses)+".png");

					ofImage img;
					img.load("outputs\\" + ofToString(numPoses)+".png");
					poseImages.push_back(img);

					learnedPoses.begin();
					ofClear(0);
					learnedPoses.end();

                    chainevent.next();
                }


				feedBackFbo.begin();
				font.drawString("Get in position in "+ofToString(5.0 - chainevent.getTime(), 0)+" seconds!", messageX, messageY);
				ofDrawRectangle(0, ofGetHeight() - 50, (chainevent.getTime() / 5.0)*ofGetWidth(), 50);
				feedBackFbo.end();

                break;
            }
            case LEARNING: {
                
                classifier.addSample(sample, numPoses);
                int drinkBeingLearned = numPoses%NUM_DRINKS;

                        learnedPoses.begin();
						ofSetColor(255, 255, 255, 100);
						ofNoFill();
						for (int i = 0; i <MIN(numHumnas, theUsers.size()); i++)theUsers[i].draw();
                        learnedPoses.end();
 
                
                if (chainevent.getTime()>chainevent.getDuration() - 0.5) {
                    
                    poseMap[drinkBeingLearned] = numPoses;
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
                pose = classifier.predict(sample);

                drink = -1;
                for (int i = 0; i < NUM_DRINKS; i++) {
                    if (pose == poseMap[i]) {
                        chainevent.next();
                        drink = i;
						break;
                    }
                }
				if (drink == -1) {
					feedBackFbo.begin();
					font.drawString("Nej!", messageX, messageY);
					feedBackFbo.end();
				}
                break;
            }
                
                
            case HIT: {
				feedBackFbo.begin();
				font.drawString("YAY! hold that pose for "+ofToString(chainevent.getDuration()-chainevent.getTime(), 0) + " seconds more!", messageX, messageY);
				feedBackFbo.end();

                pose = classifier.predict(sample);
                // cout << pose << " drink: " << poseMap[drink] << endl;
                if (pose != poseMap[drink])chainevent.back();
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
                cout << "RESET" << endl;
                chainevent.setTo(PLAYING);
                break;
            }
            default:
                break;
        }
    }
    else if(numHumnas>1){
        // cout 
		feedBackFbo.begin();
		font.drawString("I can only handle one person at a time. One of you, move away!", messageX, messageY);
		feedBackFbo.end();
	}
	else if (numHumnas == 0) {
		feedBackFbo.begin();
		font.drawString("Try the pose drink AI machine...", messageX, messageY);
		feedBackFbo.end();
	}

	userFbo.begin();
	ofFill();
	ofSetColor(255, 255, 255, 40);
	ofDrawRectangle(0,0,userFbo.getWidth(), userFbo.getHeight());
	ofSetColor(255, 255, 255);
	ofNoFill();
	for (int i = 0; i <MIN(numHumnas, theUsers.size()); i++)theUsers[i].draw();
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
	learnedPoses.draw(0, 0, learnedPoses.getWidth() / 4, learnedPoses.getHeight() / 4);
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
