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
	void setup() {
		trainingData.setNumDimensions(18 * 2);

		//setClassifier( RANDOM_FOREST_100 );
		setClassifier(NAIVE_BAYES);
		load();
		train();

	}
	int getNumCLasses() {
		return trainingData.getNumClasses();
	}
	void train() {
		if (trainingData.getNumClasses() > 2)
			pipeline.train(trainingData);
	}

	void addSample(GRT::VectorFloat sample, int label) {
		trainingData.addSample(UINT(label), sample);
	}
	void save() { trainingData.save(ofToDataPath("TrainingData.grt")); }

	bool isTrained() {
		return pipeline.getTrained();
	}


	bool doPrediction;

	void updateSample(GRT::VectorFloat s) {

		if (pipeline.getTrained() && doPrediction) {
			pipeline.predict(s);
		}
        
		doPrediction = !doPrediction;

		probability = pipeline.getMaximumLikelihood();
		label = pipeline.getPredictedClassLabel();
		//cout << label << endl;
		//cout << probability << endl;

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
        for(int i = 0; i<trainingData.getNumSamples();i++){
            if(trainingData[i].getClassLabel()==l)trainingData.removeSample(i);
        }
    }

    float probability;
    int label;
    
    void tryNewClassifier(){
        setClassifier( ++this->classifierType % NUM_CLASSIFIERS );
    }
    bool setClassifier( const int type ){
        
        AdaBoost adaboost;
        DecisionTree dtree;
        KNN knn;
        GMM gmm;
        ANBC naiveBayes;
        MinDist minDist;
        RandomForests randomForest;
        Softmax softmax;
        SVM svm;
        bool enableNullRejection = false;
        
        this->classifierType = type;
        pipeline.clear();
        
        switch( classifierType ){
            case ADABOOST:
                adaboost.enableNullRejection( enableNullRejection );
                adaboost.setNullRejectionCoeff( 3 );
                pipeline << adaboost;
                break;
            case DECISION_TREE:
                dtree.enableNullRejection( enableNullRejection );
                dtree.setNullRejectionCoeff( 3 );
                dtree.setMaxDepth( 10 );
                dtree.setMinNumSamplesPerNode( 3 );
                dtree.setRemoveFeaturesAtEachSplit( false );
                pipeline << dtree;
                break;
            case KKN:
                knn.enableNullRejection( enableNullRejection );
                knn.setNullRejectionCoeff( 3 );
                pipeline << knn;
                break;
            case GAUSSIAN_MIXTURE_MODEL:
                gmm.enableNullRejection( enableNullRejection );
                gmm.setNullRejectionCoeff( 3 );
                pipeline << gmm;
                break;
            case NAIVE_BAYES:
                naiveBayes.enableNullRejection( enableNullRejection );
                naiveBayes.setNullRejectionCoeff( 3 );
                pipeline << naiveBayes;
                break;
            case MINDIST:
                minDist.enableNullRejection( enableNullRejection );
                minDist.setNullRejectionCoeff( 3 );
                pipeline << MinDist( false, true );
                break;
            case RANDOM_FOREST_10:
                randomForest.enableNullRejection( enableNullRejection );
                randomForest.setNullRejectionCoeff( 3 );
                randomForest.setForestSize( 10 );
                randomForest.setNumRandomSplits( 2 );
                randomForest.setMaxDepth( 10 );
                randomForest.setMinNumSamplesPerNode( 5 );
                randomForest.setRemoveFeaturesAtEachSplit( false );
                pipeline << randomForest;
                break;
            case RANDOM_FOREST_100:
                randomForest.enableNullRejection( enableNullRejection );
                randomForest.setNullRejectionCoeff( 3 );
                randomForest.setForestSize( 100 );
                randomForest.setNumRandomSplits( 2 );
                randomForest.setMaxDepth( 10 );
                randomForest.setMinNumSamplesPerNode( 3 );
                randomForest.setRemoveFeaturesAtEachSplit( false );
                pipeline << randomForest;
                break;
            case RANDOM_FOREST_200:
                randomForest.enableNullRejection( enableNullRejection );
                randomForest.setNullRejectionCoeff( 3 );
                randomForest.setForestSize( 200 );
                randomForest.setNumRandomSplits( 2 );
                randomForest.setMaxDepth( 10 );
                randomForest.setMinNumSamplesPerNode( 3 );
                randomForest.setRemoveFeaturesAtEachSplit( false );
                pipeline << randomForest;
                break;
            case SOFTMAX:
                softmax.enableNullRejection( enableNullRejection );
                softmax.setNullRejectionCoeff( 3 );
                pipeline << softmax;
                break;
            case SVM_LINEAR:
                svm.enableNullRejection( enableNullRejection );
                svm.setNullRejectionCoeff( 3 );
                pipeline << SVM(SVM::LINEAR_KERNEL);
                break;
            case SVM_RBF:
                svm.enableNullRejection( enableNullRejection );
                svm.setNullRejectionCoeff( 3 );
                pipeline << SVM(SVM::RBF_KERNEL);
                break;
            default:
                return false;
                break;
                
                
        }
        train();
        currentClassifier = classifierTypeToString( classifierType );
        return true;
    }
    string currentClassifier;
    
private:
    ClassificationData trainingData;              //This will store our training data
    GestureRecognitionPipeline pipeline;
    enum ClassifierType{ ADABOOST=0, DECISION_TREE, KKN, GAUSSIAN_MIXTURE_MODEL, NAIVE_BAYES, RANDOM_FOREST_100, RANDOM_FOREST_10, MINDIST, RANDOM_FOREST_200, SOFTMAX, SVM_LINEAR, SVM_RBF, NUM_CLASSIFIERS };
    int classifierType;
    
    string classifierTypeToString( const int type ){
        switch( type ){
            case ADABOOST:
                return "ADABOOST";
                break;
            case DECISION_TREE:
                return "DECISION_TREE";
                break;
            case KKN:
                return "KKN";
                break;
            case GAUSSIAN_MIXTURE_MODEL:
                return "GMM";
                break;
            case NAIVE_BAYES:
                return "NAIVE_BAYES";
                break;
            case MINDIST:
                return "MINDIST";
                break;
            case RANDOM_FOREST_10:
                return "RANDOM_FOREST_10";
                break;
            case RANDOM_FOREST_100:
                return "RANDOM_FOREST_100";
                break;
            case RANDOM_FOREST_200:
                return "RANDOM_FOREST_200";
                break;
            case SOFTMAX:
                return "SOFTMAX";
                break;
            case SVM_LINEAR:
                return "SVM_LINEAR";
                break;
            case SVM_RBF:
                return "SVM_RBF";
                break;
        }
        return "UNKOWN_CLASSIFIER";
    }

};
#endif /* learner_h */
