//
//  learner.h
//  summerparty
//
//  Created by Karina Korsgaard Jensen on 04/06/2018.
//

#ifndef learner_h
#define learner_h
#include "ofxGrt.h"

class GestureLearner{
public:
    void setup(){
        trainingData.setNumDimensions( 18*2 );
        
        ANBC naiveBayes;
        bool enableNullRejection = false;
        naiveBayes.enableNullRejection( enableNullRejection );
        naiveBayes.setNullRejectionCoeff( 3 );
        pipeline << naiveBayes;
    }
    void train(){pipeline.train( trainingData );}
    
    void addSample(GRT::VectorFloat sample, int label){
        trainingData.addSample( UINT(label), sample );
    }
    void save(){trainingData.save( ofToDataPath("TrainingData.grt") );}
    
    bool isTrained(){
        return pipeline.getTrained();
    }
    void predict(GRT::VectorFloat sample){
        if( pipeline.getTrained() ){
            pipeline.predict( sample );
            probability = pipeline.getMaximumLikelihood();
            label = pipeline.getPredictedClassLabel();
        }
    }

    void drawAllLabels(){
        for(int i = 0; i < trainingData.getNumSamples(); i++){
            int l = trainingData[i].getClassLabel();
            ofPushMatrix();
            ofTranslate((ofGetWidth()/5)*(l%5), (ofGetHeight()/5)*int(l/5));
            
            for(int u = 0; u<trainingData.getNumDimensions(); u+=2){
                ofDrawCircle(trainingData[i][u]*(ofGetWidth()/5), trainingData[i][u+1]*(ofGetHeight()/5), 5);
            }
            
            ofPopMatrix();
        }
        ofNoFill();
        
        for(int i = 0; i<25; i++){
            ofPushMatrix();
            ofTranslate((ofGetWidth()/5)*(i%5), (ofGetHeight()/5)*int(i/5));
            ofDrawRectangle(0, 0, ofGetWidth()/5, ofGetHeight()/5);
            ofPopMatrix();
        }
    }
    
    void load(){
        trainingData.load( ofToDataPath("TrainingData.grt"));
    }
    
    void clearSample(int l){
        trainingData.removeSample(l);
    }

    float probability;
    int label;
private:
    ClassificationData trainingData;              //This will store our training data
    GestureRecognitionPipeline pipeline;
    enum ClassifierType{ ADABOOST=0, DECISION_TREE, KKN, GAUSSIAN_MIXTURE_MODEL, NAIVE_BAYES, MINDIST, RANDOM_FOREST_10, RANDOM_FOREST_100, RANDOM_FOREST_200, SOFTMAX, SVM_LINEAR, SVM_RBF, NUM_CLASSIFIERS };

};
#endif /* learner_h */
