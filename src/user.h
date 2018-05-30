//
//  user.h
//  summerparty
//
//  Created by Karina Korsgaard Jensen on 29/05/2018.
//

#ifndef user_h
#define user_h

class user {
public:

	int userId;
	float maxX;
	float maxY;

	vector<ofPoint>points;
	vector<ofPoint>particles;
	vector<ofImage>imgs;

	vector<vector<int>>connects = {
		{ 10,9,8,1,11,12,13 },
		{ 4,3,2,1,5,6,7 },
		{ 16,14,0,15,17 },
		{ 0,1 }
	};

	void setup() {
		points.resize(18);
		particles.resize(18);

		ofImage img;
		img.load("jorge/head.png");
		imgs.push_back(img);
		img.load("jorge/body.png");
		imgs.push_back(img);
		img.load("jorge/leg_upper.png");
		imgs.push_back(img);
		img.load("jorge/leg_lower.png");
		imgs.push_back(img);
		img.load("jorge/arm_upper.png");
		imgs.push_back(img);
		img.load("jorge/arm_lower.png");
		imgs.push_back(img);
	}

	void update() {
		for (int i = 0; i<connects.size(); i++) {
			for (int u = 0; u < connects[i].size(); u++) {
				int p = connects[i][u];
				

				if (points[p].x == 0) {
					if (u != connects[i].size() - 1) {
						int pp = connects[i][u + 1];
						if (points[pp].x > 0)
							particles[p] = particles[p].getInterpolated(points[pp], .1);
					}
					if (u != 0) {
						int pm = connects[i][u - 1];
						if (points[pm].x > 0)
							particles[p] = particles[p].getInterpolated(points[pm], .1);
					}
				}
			}
		}
	}
	void drawStomach() {
		ofPath p;
#ifdef RUNWAY
		ofVec2f scale = ofVec2f(400, 400);
#else
		ofVec2f scale = ofVec2f(0.5, 0.5);
#endif


		
		ofVec2f p1 = (points[2] + points[5]) / 2;
		ofVec2f p2 = (points[11] + points[8]) / 2;
		ofVec2f mean = (p1 + p2) / 2;
		float orientation = getRotation(p1, p2);
		float sizex = points[2].x - points[5].x;
		float sizey = p1.y - p2.y;
		
		ofPushMatrix();
		ofTranslate(mean.x * scale.x, mean.y * scale.y);
		ofRotate(90+orientation);
		imgs[1].draw(-sizex/2, -sizey/2, sizex, sizey);
		ofPopMatrix();
		
		float aspect = sizex / imgs[1].getWidth();

		ofPushMatrix();
		ofTranslate(points[0] * scale);
		ofRotate(180);
		imgs[0].draw(-imgs[0].getWidth()*aspect / 2, -imgs[0].getHeight()*aspect , imgs[0].getWidth()*aspect, imgs[0].getHeight()*aspect);
		ofPopMatrix();
/*
		mean = (points[2] + points[3]) / 2;
		orientation = getRotation(points[5], points[6]);
		ofPushMatrix();
		ofTranslate(mean.x * scale.x, mean.y * scale.y);
		ofRotate(90 + 180 + orientation);
		drawImage(4, 20, (points[2] - points[3]).length());
		ofPopMatrix();

		mean = (points[3] + points[4]) / 2;
		orientation = getRotation(points[6], points[7]);
		ofPushMatrix();
		ofTranslate(mean.x * scale.x, mean.y * scale.y);
		ofRotate(90 + 180 + orientation);
		drawImage(5, 22, (points[3] - points[4]).length());
		ofPopMatrix();

		mean = (points[5] + points[6]) / 2;
		orientation = getRotation(points[5] , points[6]);
		ofPushMatrix();
		ofTranslate(mean.x * scale.x, mean.y * scale.y);
		ofRotate(90 + 180 + orientation);
		drawImage(4, 20, (points[5] - points[6]).length());
		ofPopMatrix();

		mean = (points[6] + points[7]) / 2;
		orientation = getRotation(points[6] , points[7]);
		ofPushMatrix();
		ofTranslate(mean.x * scale.x, mean.y * scale.y);
		ofRotate(90 + 180 + orientation);
		drawImage(5, 22, (points[6]-points[7]).length());
		ofPopMatrix();
		*/
		// ofDrawCircle(points[0] * scale, 20);

	}
	void drawImage(int i, float aspect) {
		imgs[i].draw(-imgs[i].getWidth()*aspect / 2, -imgs[i].getHeight()*aspect/2, imgs[i].getWidth()*aspect, imgs[i].getHeight()*aspect);
	}
	void drawImage(int i, float w, float h) {
		imgs[i].draw(-w / 2, -h/2, w, h);
	}
	float getRotation(ofVec2f a, ofVec2f b) {
		return atan2(b.y - a.y, b.x - a.x) * 180 / PI;
	}

	void addPoint(int i, float x, float y) {
		points[i] = ofPoint((x), (y));
		if(x>0&&y>0)particles[i] = ofPoint((x), (y));
	}
	void clearPoints() { points.clear(); points.resize(18); }

	void print() {
		cout << "person" << userId << " ";
		for (auto p : particles) {
			cout << p.x << " " << p.y;
		}
		cout << " " << endl;

	}

	void draw() {

		for (int i = 0; i<connects.size(); i++) {
			for (int u = 0; u < connects[i].size() - 1; u++) {

				ofPoint p1 = particles[connects[i][u]];
				ofPoint p2 = particles[connects[i][u + 1]];


#ifdef RUNWAY
				p1 *= 400;
				p2 *= 400;
#else
				p1 *= 0.5;
				p2 *= 0.5;
#endif

				ofPoint p3 = ofPoint((p1.x + p2.x) / 2., (p2.y + p1.y) / 2.);
				float p4 = (p2 - p1).length();

				ofPushMatrix();
				ofTranslate(p3.x, p3.y);
				ofRotate(atan2(p2.y - p1.y, p2.x - p1.x) * 180 / PI);
				ofDrawRectangle(-p4 / 2.f, -5, p4, 10);
				ofPopMatrix();
			}
		}


		drawStomach();

	}
};
#endif /* user_h */
#pragma once
