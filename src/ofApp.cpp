#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    
    r.setup(7000);
    
    test.listDevices();
    test.setDeviceID(1);
    test.setup(SCALE_X, SCALE_Y);
    
    ofEnableAlphaBlending();
    ofEnableAntiAliasing();
    
    yellowShader.load("shader");
    
    for (int i = 0; i<25; i++) {
        vector<double>newAverage;
        newAverage.resize(18 * 2);
        averagePoses.push_back(newAverage);
        
        ofImage img;
        if (img.load("outputs\\" + ofToString(i) + ".png"))poseImages.push_back(img);
    }
    logo.load("logo.png");
    backgound.load("background.png");
    speech.load("speech.png");
    speech_red.load("speech_red.png");
    
    
    feedback.setup();
    float aspect = logo.getHeight()/logo.getWidth();
    feedback.addImage(&logo, WIDTH/2, 200, 817, aspect * 817, 100, 5.);
    
    aspect = backgound.getHeight()/backgound.getWidth();
    feedback.addImage(&backgound, WIDTH/2, HEIGHT - aspect*WIDTH, WIDTH, aspect*WIDTH, 100, 5.);
    
    box2d.init();
    box2d.setGravity(0, 0.0);
    box2d.setFPS(30.0);
    box2d.setIterations(10, 2);
    box2d.disableEvents();
    box2d.disableGrabbing();
    
    theUsers.resize(MAX_USERS);
    for (int i = 0; i < MAX_USERS; i++) {
        theUsers[i].setup(&box2d);
    }
    bartender.setup(&box2d);
    
    classifier.setup();
    
    chainevent.addEvent(3., TOOMANY, true);
    chainevent.addEvent(3., NOONE, true);
    chainevent.addEvent(3., BEGIN_LEARNING);
    chainevent.addEvent(3., LEARNING);
    chainevent.addEvent(20., TRAINING, true);
    chainevent.addEvent(5., PLAYING);
    chainevent.addEvent(3., POSE1, true);
    chainevent.addEvent(3., POSE2);
    chainevent.addEvent(3., POSE3);
    chainevent.addEvent(5., POUR);
    chainevent.addEvent(3., RESET);
    chainevent.addEvent(3., TRYAGAIN);
    
    
    chainevent.setTo(PLAYING);
    
    learnedPoses.allocate(WIDTH, HEIGHT, GL_RGBA);
    learnedPoses.begin();
    ofClear(0);
    learnedPoses.end();
    
    render.allocate(WIDTH, HEIGHT, GL_RGBA);
    render.begin();
    ofClear(0);
    render.end();
    
    gui.setup();
    gui.add(top.set("user_top", 0, 0, 1000));
    gui.add(yellow_box.set("yellow_box", 0, 0, 500));
    gui.add(red_box.set("yellow_red", 0, 0, 800));
    gui.add(bellyThreshold.set("bellyThreshold", 0, 0, 1));
    gui.add(prababilityThreshold.set("prababilityThreshold", 0, 0, 1));
    gui.add(addSamples.set("addsamples", false));
    gui.add(clearSample.set("clearSample", false));
    gui.add(train.set("train", false));
    
    
    bodyGroup.setName("body");
    
    
    bodyGroup.add(torso.set("torso", ofVec2f(0), ofVec2f(-10), ofVec2f(10)));
    bodyGroup.add(head.set("head", ofVec2f(0), ofVec2f(-10), ofVec2f(10)));
    bodyGroup.add(s_torso.set("s_torso", 0, 0, 1));
    bodyGroup.add(s_head.set("s_head", 0, 0, 1));
    
    for(int i = 0; i<9; i++){
        ofParameter<ofVec2f> p;
        pos_parts.push_back(p);
        bodyGroup.add(pos_parts.back().set("part_"+ofToString(i), ofVec2f(0,0), ofVec2f(-10,-10), ofVec2f(10,10)));
        
        ofParameter<float> s;
        s_parts.push_back(s);
        bodyGroup.add(s_parts.back().set("s_part_"+ofToString(i), 0, 0, 1));
    }
    
    gui.add(bodyGroup);
    
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
    dir.allowExt("png");
    dir.listDir("User");
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
        ///cout << "something" << endl;
        /* bodies / 72057594037928883 / hands / Left
         / bodies / 72057594037928883 / hands / Right
         / bodies / 72057594037928883 / joints / SpineBase
         / bodies / 72057594037928883 / joints / SpineMid
         / bodies / 72057594037928883 / joints / Neck
         / bodies / 72057594037928883 / joints / Head
         / bodies / 72057594037928883 / joints / ShoulderLeft
         / bodies / 72057594037928883 / joints / ElbowLeft
         / bodies / 72057594037928883 / joints / WristLeft
         / bodies / 72057594037928883 / joints / HandLeft
         / bodies / 72057594037928883 / joints / ShoulderRight
         / bodies / 72057594037928883 / joints / ElbowRight
         / bodies / 72057594037928883 / joints / WristRight
         / bodies / 72057594037928883 / joints / HandRight
         / bodies / 72057594037928883 / joints / HipLeft
         / bodies / 72057594037928883 / joints / KneeLeft
         / bodies / 72057594037928883 / joints / AnkleLeft
         / bodies / 72057594037928883 / joints / FootLeft
         / bodies / 72057594037928883 / joints / HipRight
         / bodies / 72057594037928883 / joints / KneeRight
         / bodies / 72057594037928883 / joints / AnkleRight
         / bodies / 72057594037928883 / joints / FootRight
         / bodies / 72057594037928883 / joints / SpineShoulder
         / bodies / 72057594037928883 / joints / HandTipLeft
         / bodies / 72057594037928883 / joints / ThumbLeft
         / bodies / 72057594037928883 / joints / HandTipRight
         / bodies / 72057594037928883 / joints / ThumbRight
         */
        //cout << m.getAddress() << endl;
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
                        // if(x > left && x < right+left && y > top && y < bottom+top){
                        //     theUsers[id].pointsInView++;
                        // cout << "true" << endl;
                        // }
                        
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
    
    feedback.update();
    
    
    
    int messageX = WIDTH/2;
    int messageY = yellow_box + 155;
    int messageRedX = WIDTH/2;
    int messageRedY = red_box + 100;
    int line1 = 75;
    int line2 = 55;
    
    
    
    if (numHumansInView>1) {
        chainevent.setTo(TOOMANY);
    }
    if (numHumans==0) {
        chainevent.setTo(NOONE);
    }
    chainevent.update();

    switch (chainevent.getName()) {
        case BEGIN_LEARNING: {
            
            if (chainevent.getDuration()-chainevent.getTime() < 0.1) {
                chainevent.next();
            }
            
            break;
        }
        case LEARNING: {
            user u = theUsers[userInView];
            GRT::VectorFloat sample = getSample(u);
            
            classifier.addSample(sample, numPoses);
            std::transform (averagePoses[numPoses].begin(), averagePoses[numPoses].end(), sample.begin(), averagePoses[numPoses].begin(), std::plus<double>());
            numSamples ++;
            
            
            
            if (chainevent.getTime()>chainevent.getDuration() - 0.1) {
                
                std::transform(averagePoses[numPoses].begin(), averagePoses[numPoses].end(), averagePoses[numPoses].begin(),
                               std::bind1st(std::multiplies<double>(),1.0/double(numSamples+1)));
                
                learnedPoses.begin();
                ofClear(0);
                ofFill();
                ofSetColor(254, 185, 24);
                drawUserWithPngs(u.circlePoints, 0);
                learnedPoses.end();
                
                ofPixels pix;
                learnedPoses.readToPixels(pix);
                ofSaveImage(pix, "outputs\\" + ofToString(numPoses) + ".png");
                
                ofDisableArbTex();
                ofImage img;
                img.load("outputs\\" + ofToString(numPoses) + ".png");
                ofEnableArbTex();
                
                
                
                yellowShader.begin();
                learnedPoses.begin();
                ofClear(0);
                yellowShader.setUniformTexture("tex0", img.getTexture(), 1);
                ofSetColor(255);
                learnedPoses.draw(0, 0, WIDTH, HEIGHT);
                learnedPoses.end();
                yellowShader.end();
                
                
                learnedPoses.readToPixels(pix);
                ofSaveImage(pix, "outputs\\" + ofToString(numPoses) + ".png");
                img.load("outputs\\" + ofToString(numPoses) + ".png");
                if (numPoses >= poseImages.size())
                    poseImages.push_back(img);
                else poseImages[numPoses] = img;
                
                numSamples = 0;
                chainevent.next();
                
            }
            break;
        }
            
            
        case TRAINING: {
            classifier.train();
            chainevent.next();
            break;
            
        }
        case PLAYING: {
            user u = theUsers[userInView];
            GRT::VectorFloat sample = getSample(u);
            
            if(chainevent.isfirstframe){
                feedback.addImage(&speech, WIDTH/2, yellow_box, 763, 389, 1);
                feedback.addText("¡Preparar!", messageX, messageY, 2, true, 1., ofColor(34,38,76));
                feedback.addText("Hit all three swag poses to", messageX, messageY + line1, 2, false, 1., ofColor(34,38,76));
                feedback.addText("open the drink valve", messageX, messageY + line1 + line2, 2, false, 1., ofColor(34,38,76));
                chainevent.isfirstframe = false;
            }
            break;
        }
        case POSE1: {
            user u = theUsers[userInView];
            GRT::VectorFloat sample = getSample(u);
            if(chainevent.isfirstframe){
                //feedback.addImage(speech,WIDTH/2 - 763/2, yellow_box, 763, 389, 1);
                feedback.removeDrawable(2, 0.5);
                feedback.addText("¡Plantear Uno!", messageX, messageY, 2, true, 1., ofColor(34,38,76));
                feedback.addText("Flamingo, Flamingo", messageX, messageY + line1, 2, false, 1., ofColor(34,38,76));
                //feedback.addText("open the drink valve", messageX, messageY + 60 + 40, 2, false, 1., ofColor(34,38,76));
                chainevent.isfirstframe = false;
            }
            
            currentDrinkSequence = 0;
            classifier.updateSample(sample);
            int currentPose = classifier.label;
            
            /* if(int(chainevent.getTime())%10 == 1){
             //feedback.removeDrawable(2, 0.5);
             feedback.addImage(red_box,WIDTH/2, red_box, 763, 389, 3);
             feedback.addText("¡Incorrecto!", messageX, messageY, 3, true, 1., ofColor(34,38,76));
             //feedback.addText("Flamingo, Flamingo", messageX, messageY + 60, 2, false, 1., ofColor(34,38,76));
             //feedback.addText("open the drink valve", messageX, messageY + 60 + 40, 2, false, 1., ofColor(34,38,76));
             chainevent.isfirstframe = false;
             }
             if(int(chainevent.getTime())%10 == 7){
             feedback.removeDrawable(3, 0.5);
             }
             */
            
            if (currentPose == drinkSequence[currentDrinkSequence] && classifier.probability > prababilityThreshold)
            {
                feedback.removeDrawable(2, 0.5);
                feedback.addText("¡Correcto!", messageX, messageY, 2, true, 1., ofColor(34,38,76));
                feedback.addText("Do the next pose!", messageX, messageY + line1, 2, false, 1., ofColor(34,38,76));
                chainevent.isfirstframe = false;
                
                currentDrinkSequence ++;
                test.update();
                ofSaveImage(test.getPixels(), "images\\session_"+ofToString(session)+"1.png");
                chainevent.next();
            }
            
            break;
        }
            
            
        case POSE2: {
            user u = theUsers[userInView];
            GRT::VectorFloat sample = getSample(u);
            if(chainevent.isfirstframe){
                feedback.removeDrawable(2, 0.5);
                feedback.addText("¡Next pose! - "+ofToString(chainevent.getDuration()- chainevent.getTime(), 1), messageX, messageY, 2, true, 1., ofColor(34,38,76));
                chainevent.isfirstframe = false;
            }
            
            
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
        case POSE3: {
            user u = theUsers[userInView];
            GRT::VectorFloat sample = getSample(u);
            if(chainevent.isfirstframe){
                feedback.removeDrawable(2, 0.5);
                feedback.addText("¡Final one! - "+ofToString(chainevent.getDuration()- chainevent.getTime(), 1), messageX, messageY, 2, true, 1., ofColor(34,38,76));
                chainevent.isfirstframe = false;
            }
            
            
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
            if(chainevent.isfirstframe){
                feedback.removeDrawable(2, 0.5);
                feedback.addText("¡Hurra!", messageX, messageY, 2, true, 1., ofColor(34,38,76));
                feedback.addText("Your drink is being poured!", messageX, messageY + line1, 2, false, 1., ofColor(34,38,76));
                chainevent.isfirstframe = false;
            }
            serial.writeByte('o');
            break;
        }
        case RESET: {
            if(chainevent.isfirstframe){
                feedback.removeDrawable(2, 0.5);
                feedback.removeDrawable(1, 0.5);
            }
            serial.writeByte('c');
            
            int i = ofRandom(classifier.getNumCLasses());
            int j = ofRandom(classifier.getNumCLasses() -1);
            int k = ofRandom(classifier.getNumCLasses() -2);
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
            
            if(chainevent.isfirstframe){
                feedback.removeDrawable(2, 0.5);
                feedback.addText("¡Failed!", messageX, messageY, 2, true, 1., ofColor(34,38,76));
                feedback.addText("Try again..!", messageX, messageY + line1, 2, false, 1., ofColor(34,38,76));
                chainevent.isfirstframe = false;
            }
            
            if (chainevent.getDuration() - chainevent.getTime() < 0.1)chainevent.setTo(PLAYING);
            break;
        }
        case TOOMANY: {
            if(chainevent.isfirstframe){
                feedback.removeDrawable(2, 0.5);
                feedback.removeDrawable(1, 0.5);
                feedback.addImage(&speech,WIDTH/2, yellow_box, 763, 389, 1);
                feedback.addText("¡Uno por favor!", messageX, messageY, 2, true, 1., ofColor(34,38,76));
                feedback.addText("One of you, step away!", messageX, messageY + line1, 2, false, 1., ofColor(34,38,76));
                chainevent.isfirstframe = false;
            }
        }
        case NOONE: {
            if(chainevent.isfirstframe){
                feedback.removeDrawable(2, 0.5);
                feedback.removeDrawable(1, 0.5);
                feedback.addImage(&speech, WIDTH/2, yellow_box, 763, 389, 1);
                feedback.addText("¡Hola Senor!", messageX, messageY, 2, true, 1., ofColor(34,38,76));
                feedback.addText("Step right up to", messageX, messageY + line1, 2, false, 1., ofColor(34,38,76));
                feedback.addText("get your drink", messageX, messageY + line1 + line2, 2, false, 1., ofColor(34,38,76));
                
                chainevent.isfirstframe = false;
            }
        }
        default:
            break;
    }
    

    
    
    
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

//--------------------------------------------------------------
void ofApp::draw() {
    
    
    render.begin();
    ofClear(0);
    ofPushMatrix();
    ofTranslate(0, top);
    if (drinkSequence[currentDrinkSequence]<poseImages.size())poseImages[drinkSequence[currentDrinkSequence]].draw(0, 0, WIDTH, HEIGHT);
    for (int i = 0; i < MIN(numHumans, theUsers.size()); i++) {
        drawUserWithPngs(theUsers[i].circlePoints, drawUserOrbartender);
    }
    ofPopMatrix();
   
    feedback.draw(0, 0);
    render.end();
    
    
    ofPushMatrix();
    ofScale(0.3f, 0.3f);
    ofSetHexColor(0x22264C);
    ofDrawRectangle(0, 0, WIDTH, HEIGHT);
    ofSetColor(255);
    render.draw(0, 0);
    ofPopMatrix();
    
    gui.draw();
    
    
    if(debug){
        classifier.drawAllLabels();
        ofNoFill();
        ofSetLineWidth(5);
        ofPushMatrix();
        ofTranslate((ofGetWidth()/5)*(numPoses%5), (ofGetHeight()/5)*int(numPoses/5));
        ofDrawRectangle(0, 0, ofGetWidth()/5, ofGetHeight()/5);
        ofPopMatrix();
        ofSetLineWidth(1);
        
        ofDrawBitmapString("Classifier: " + classifier.currentClassifier, gui.getWidth() + 40, 10);
    }
}
void ofApp::drawUserWithPngs(vector<ofVec2f> p, int pngs){
    
    // torso
    ofVec2f mean = (p[2] + p[5] + p[8] + p[11]) / 4;
    ofPushMatrix();
    ofTranslate(mean.x , mean.y );
    ofRotate(getAngle(p[1], p[14]) - 90);
    float w = (p[5] - p[2]).length() ;
    float h = (p[1] - getMean(p[8], p[11])).length() ;
    int img = 13;
    if (w / h < bellyThreshold)img = 1;
    w = MAX(30, w);
    //cout << w << " " << h << endl;
    
    bodyPartImages[img + pngs*14].draw(-w / 2, -h / 2, w, h);
    ofPopMatrix();
    
    vector<vector<int>> indx = { { 4,3,11 },{ 3,2,0 },{ 5,6,0 },{ 7,6,12 },{ 8,9,0 },{ 10,9,9 },{ 11,12,0 },{ 13,12,10 } };
    
    for (int i = 0; i < indx.size(); i++) {
        float rotation = getAngle(p[indx[i][0]], p[indx[i][1]]);
        ofVec2f position = getMean(p[indx[i][0]], p[indx[i][1]]);
        float w = bodyPartImages[i + pngs * 14].getWidth();
        float h = (p[indx[i][1]] - p[indx[i][0]]).length();
        ofPushMatrix();
        ofTranslate(position.x + pos_parts[i].get().x, position.y + pos_parts[i].get().y);
        ofRotate(rotation - 90);
        bodyPartImages[i + pngs*14].draw(-(w*s_parts[i]) / 2, -(h)/2, w*s_parts[i], h);
        ofPopMatrix();
        
        if(indx[i][2]>0) {
            h = bodyPartImages[indx[i][2] + pngs * 14].getHeight();
            w = bodyPartImages[indx[i][2] + pngs * 14].getWidth();
            ofPushMatrix();
            ofTranslate(p[indx[i][0]].x , p[indx[i][0]].y );
            ofRotate(rotation - 90);
            bodyPartImages[indx[i][2] + pngs*14].draw(-(w*s_parts[8]/2), -(h*s_parts[8]/2), w*s_parts[8], h*s_parts[8]);
            ofPopMatrix();
        }
    }
    
    //head
    ofPushMatrix();
    ofTranslate(getMean(p[0], p[1]).x , getMean(p[0], p[1]).y );
    ofRotate(getAngle(p[0], p[1])-90);
    bodyPartImages[8 + pngs*14].draw(-(bodyPartImages[8+ pngs*14].getWidth()*s_head) / 2, -(bodyPartImages[8+ pngs*14].getHeight()*s_head)/2, bodyPartImages[8+ pngs*14].getWidth()*s_head, bodyPartImages[8+ pngs*14].getHeight()*s_head);
    ofPopMatrix();
    
    echoArduino();
    if(serial.isInitialized())readArduino();
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if (key == 'l')chainevent.beginEvents();
    if (key == 'd')debug = !debug;
    if (key == 's')classifier.save();
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    if(key == OF_KEY_TAB){
        classifier.tryNewClassifier();
    }
    if (key == 'b')drawUserOrbartender = !drawUserOrbartender;
}
void ofApp::echoArduino() {
    
    echoTimer += ofGetLastFrameTime();
    if(echo && echoTimer > 30.){
        if(serial.isInitialized())serial.writeByte('q');
        echo = false;
        echoTimer = 0.0;
    }
    if(echoTimer>5. && !echo) {
        serial.listDevices();
        vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
        echoTimer = 0.0;
        int baud = 9600;
        if(deviceList.size()>0)
            serial.setup(deviceList[deviceCount%deviceList.size()].getDeviceName(), baud);
        
        nTimesRead = 0;
        nBytesRead = 0;
        readTime = 0;
        memset(bytesReadString, 0, 4);
        
        if(serial.isInitialized())
            serial.writeByte('q');
        
        deviceCount++;
        cout<<"no echo"<<endl;
    }
}

void ofApp::readArduino(){
    
    int tempInput = -1;
    nTimesRead = 0;
    nBytesRead = 0;
    int nRead  = 0;  // a temp variable to keep count per read
    
    unsigned char bytesReturned[3];
    
    memset(bytesReadString, 0, 4);
    memset(bytesReturned, 0, 3);
    
    while( (nRead = serial.readBytes( bytesReturned, 3)) > 0){
        nTimesRead++;
        nBytesRead = nRead;
    };
    
    //if(nBytesRead>0){
    memcpy(bytesReadString, bytesReturned, 3);
    
    bSendSerialMessage = false;
    readTime = ofGetElapsedTimef();
    
    string fromArduino = string(bytesReadString);
    
    if(fromArduino == "w") {
        echo = true;
        cout<< "arduino is on"<<endl;
    }
    
    char fa = fromArduino[0];
    tempInput = fa;
    
    if(tempInput>0) {
        echoTimer = 0.0;
        echo = true;
    }
    
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
