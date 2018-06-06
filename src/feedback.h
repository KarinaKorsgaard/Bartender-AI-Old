//
//  feedback.h
//  summerparty
//
//  Created by Karina Korsgaard Jensen on 06/06/2018.
//

#ifndef feedback_h
#define feedback_h
#define WIDTH 1080
#define HEIGHT 1920
struct drawable{
    ofImage* img;
    bool isImage = false;
    double timer = 0.0;
    double transitiontime = 1.0;
    float alpha = 0.f;
    string text;
    bool isLarge = false;
    bool remove = false;
    int iid;
    ofColor color;
    int w, h, x, y;
};
class Feedback{

    ofTrueTypeFont small;
    ofTrueTypeFont large;
    
public:
    vector<drawable>drawables;
    
    void setup(){
        small.load("Apercu-Bold.ttf", 40);
        large.load("Apercu-Medium.ttf", 66);
    }
    void addImage(ofImage * img, int x, int y, int w, int h, int _iid, float trans = 1., ofColor col = ofColor(255)){
        drawable d;
        d.img = img;
        d.isImage = true;
        d.transitiontime = trans;
        d.iid = _iid;
        d.color = col;
        d.w = w;
        d.h = h;
        d.x = x;
        d.y = y;
        drawables.push_back(d);
    }
    void addText(string str, int x, int y, int _iid, bool isLarge, float trans = 1., ofColor col = ofColor(255)){
        drawable d;
        d.text = str;
        d.x = x;
        d.y = y;
        d.isLarge = isLarge;
        d.transitiontime = trans;
        d.iid = _iid;
        d.color = col;
        
        drawables.push_back(d);
    }
    void removeDrawable(int _iid, double trans = 1.){
        for(int i = 0; i<drawables.size(); i++){
            if(drawables[i].iid == _iid){
                drawables[i].remove = true;
                drawables[i].timer = 0.0;
                drawables[i].transitiontime = trans;
            }
        }
    }
    void update(){
        for(int i = 0; i<drawables.size(); i++){
            drawables[i].timer+=ofGetLastFrameTime();
            if(drawables[i].remove){
                drawables[i].alpha = ease(drawables[i].timer, 255, 0, drawables[i].transitiontime);
                if(drawables[i].alpha<0.1)drawables.erase(drawables.begin()+i);
            }
            else drawables[i].alpha = ease(drawables[i].timer, 0, 255, drawables[i].transitiontime);
        }
    }
    
    void draw(int x, int y){
        
        ofPushMatrix();
        ofTranslate(x ,y);

        
        for(int i = 0; i<drawables.size(); i++){
            ofSetColor(drawables[i].color, drawables[i].alpha);
            if(drawables[i].isImage){
                drawables[i].img->draw(drawables[i].x-(drawables[i].w/2), drawables[i].y, drawables[i].w, drawables[i].h);
            }
            else if(drawables[i].isLarge){
                large.drawString(drawables[i].text, -large.getStringBoundingBox(drawables[i].text, 0, 0).width/2 + drawables[i].x, drawables[i].y);
            }
            else{
                small.drawString(drawables[i].text, -small.getStringBoundingBox(drawables[i].text, 0, 0).width/2 + drawables[i].x, drawables[i].y);
            }
        }
        ofPopMatrix();

    }

    float ease(float time, float begin, float end, float duration) {
        end -=begin;
        
        if (time > duration) return end+begin;
        time /= duration;
        return end*time*time + begin;   
    }
};
#endif /* feedback_h */
