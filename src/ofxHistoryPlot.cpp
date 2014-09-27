/*
 *  ofxHistoryPlot.cpp
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 28/01/11.
 *  Copyright 2011 uri.cat. All rights reserved.
 *
 */

#include "ofxHistoryPlot.h"
#include <float.h>

ofxHistoryPlot::ofxHistoryPlot(float maxHistory, bool autoUpdate_){
	
	autoUpdate = autoUpdate_;
    
	MAX_HISTORY = maxHistory;
	manualRange = false;
	onlyLowestIsFixed = false;
	count = 1;
	precision = 2;
	colorSet = true;
	lineWidth = 1.0f;
	drawSkip = 1;
	showNumericalInfo = true;
	respectBorders = true;
	drawBackground = true;
	bgColor = ofColor(0);
	gridColor = ofColor(255,16);
	drawGrid = true;
	shrinkBackInAutoRange = false;
	plotNeedsRefresh = true;
	gridMesh.setMode(OF_PRIMITIVE_LINES);
	gridUnit = 40;
	smoothFactor = 0.1;
	smoothValue = 0;
	showSmoothedPlot = false;
}

ofxHistoryPlot::ofxHistoryPlot(float * val, string varName, float maxHistory, bool autoUpdate_){
    add(val, varName);
    ofxHistoryPlot(maxHistory, autoUpdate_);
}

void ofxHistoryPlot::add(float * val, string varName){
    
    ofColor & c = values.back().lineColor;
    c.setHsb(ofRandom(1), 0.5+ofRandom(0.5), 0.7);
    add(val, varName, c);
}

void ofxHistoryPlot::add(float * val, string varName, ofColor lineColor){
    values.push_back( Value(val, varName, lineColor) );
    //    if (val != NULL){
    //        lowest = *valf * 0.99;
    //        highest = *valf * 1.01;
    //    }else{
    //        lowest = -1;
    //        highest = 1;
    //    }
}

void ofxHistoryPlot::setMaxHistory(int max){
	MAX_HISTORY = max;
}

void ofxHistoryPlot::reset(){
	values.clear();
	count = 0;
}

void ofxHistoryPlot::update(/*float newVal*/){
    
    for(int j =0; j<values.size();j++){
        
        Value & v = values[j];
        float newVal = *v.valf;
        
        if (count <= 1){
            smoothValue = *v.valf;
        }

        count++;
        
//        if (newVal != newVal && valf != NULL)	//if no value is supplied (default value NAN), use the float* we were given..
//            newVal = *valf;	
        
        if ( ( manualRange && onlyLowestIsFixed ) || !manualRange ){	//update graph range every now and then
            int skip = 1;
            if(shrinkBackInAutoRange){
                //if (!autoUpdate) skip = 1;	//if not doing this too fast, no need to skip range processing
                if ( count%skip == 0 ){			
                    if (!onlyLowestIsFixed) lowest = FLT_MAX;
                    highest = -FLT_MIN;
                    for (int i = 0; i < v.values.size(); i+=skip){
                        float val = v.values[i];
                        if (val > highest) highest = val;
                        if (!onlyLowestIsFixed) if (val < lowest) lowest = val;
                    }	
                    if (lowest == FLT_MAX) lowest = -1;
                    if (highest == -FLT_MIN) highest = 1;
                }
            }
            if ( newVal > highest) highest = newVal;
            if ( newVal < lowest && !onlyLowestIsFixed) lowest = newVal;
        }

        v.values.push_back(newVal);

        if(showSmoothedPlot) {
            smoothValue = newVal * smoothFactor + smoothValue * (1.0f - smoothFactor);
            v.smoothValues.push_back(smoothValue);
        }

        if (v.values.size() > MAX_HISTORY){
            v.values.erase( v.values.begin() );
        }

        if(showSmoothedPlot) {
            if (v.smoothValues.size() > MAX_HISTORY){
                v.smoothValues.erase( v.smoothValues.begin() );
            }
        }

        plotNeedsRefresh = true;
    //	float adapt = 0.05;
    //	lowest = lowest + adapt * (newVal - lowest);
    //	highest = highest - adapt * (highest - newVal);
    }
}

void ofxHistoryPlot::draw(float x, float y){
	draw(x,y,DEFAULT_WIDTH,DEFAULT_HEIGHT);
}

void ofxHistoryPlot::draw(int x, int y){
	draw((float)x,(float)y,DEFAULT_WIDTH,DEFAULT_HEIGHT);
}

void ofxHistoryPlot::refillGridMesh(float x, float y , float w, float h){

	gridMesh.clear();
	int gridH = gridUnit;
	float numLinesH = h / gridH;
	gridMesh.setMode(OF_PRIMITIVE_LINES);
	for(int i = 0; i < numLinesH; i++){
		gridMesh.addVertex( ofVec2f(x,  y + gridH * i) );
		gridMesh.addVertex( ofVec2f(x + w,  y + gridH * i) );
	}
	float numLinesW = w / gridH;
	for(int i = 0; i < numLinesW; i++){
		gridMesh.addVertex( ofVec2f( floor(gridH * 0.5) + x + gridH * i, y ) );
		gridMesh.addVertex( ofVec2f( floor(gridH * 0.5) + x + gridH * i, y + h) );
	}
}


void ofxHistoryPlot::refillPlotMesh(ofVboMesh& mesh, vector<float> & vals, float x, float y , float w, float h){

	mesh.clear();
	float border = respectBorders ? 12 : 0;
	int start = 0;
	if (count >= MAX_HISTORY){
		start = drawSkip - (count) % (drawSkip);
	}

	float loww = lowest - 0.001f;
	float highh = highest + 0.001f;

	for (int i =  start; i < vals.size(); i+= drawSkip){
		float xx = ofMap(i, 0, MAX_HISTORY, 0, w, false);
		float yy = ofMap( vals[i], loww, highh, border, h - border, true);
		mesh.addVertex(ofVec3f(x + xx, y + h - yy));
	}
}


void ofxHistoryPlot::addHorizontalGuide(float yval, ofColor c){
	horizontalGuides.push_back(yval);
	plotNeedsRefresh = true;
	horizontalGuideColors.push_back(c);
}


void ofxHistoryPlot::clearHorizontalGuides(){
	horizontalGuides.clear();
	plotNeedsRefresh = true;
	horizontalGuideColors.clear();
}


void ofxHistoryPlot::draw(float x, float y , float w, float h){

	bool needsMesh = false;
	ofRectangle r = ofRectangle(x,y,w,h);
	if ( r != prevRect || autoUpdate || plotNeedsRefresh){
		needsMesh = true;
		plotNeedsRefresh = false;
	}

	if (autoUpdate) update();
	bool haveData = true;
	if (values.size() == 0 ) haveData = false;
	
	#ifndef TARGET_OPENGLES	
	glPushAttrib(GL_CURRENT_BIT);
	#endif
		if (drawBackground){
			ofSetColor(bgColor);
			ofRect(x, y, w, h);
			if (drawGrid){
				if(needsMesh){
					refillGridMesh(x, y, w, h);
				}
				ofSetColor(gridColor);
				#ifndef TARGET_OPENGLES
				glPushAttrib(GL_ENABLE_BIT);
				#endif
				ofSetLineWidth(1);
				gridMesh.draw();
				#ifndef TARGET_OPENGLES
				glPopAttrib();
				#endif
			}
		}
    
        for (int j=0; j<values.size(); j++) {
            Value & v = values[j];
        
            if ( showNumericalInfo && haveData){
                glColor4f(0.7,0.7,0.7,1);
                float cVal = v.values[v.values.size()-1];
                string text = v.varName + " " + ofToString(cVal, precision);
                ofDrawBitmapString(text, x + w - (text.length()) * 8  , y + 10 + j * 15);
            }
            if ( showNumericalInfo ){
                glColor4f(0.33,0.33,0.33, 1);
                ofDrawBitmapString(ofToString(highest, precision), 1 + x , y + 10);
                ofDrawBitmapString(ofToString(lowest, precision), 1 + x , y + h - 1);
            }
        }
    
        for(int i = 0; i < horizontalGuides.size(); i++){
            float myY = horizontalGuides[i];
            if (myY > lowest && myY < highest){ //TODO negative!
                float yy = ofMap( myY, lowest, highest, 0, h, true);
                ofSetColor(horizontalGuideColors[i], 100);
                ofDrawBitmapString(ofToString(horizontalGuides[i], precision), 10 * i + x, y + h - yy + 10 );
                ofSetColor(horizontalGuideColors[i], 128 );
                ofLine( x, y + h - yy, x + w, y + h - yy );
            }
        }

		
	#ifndef TARGET_OPENGLES	
	glPopAttrib();
	#endif
	
	if (haveData){
		ofNoFill();

		#ifndef TARGET_OPENGLES	
		glPushAttrib(GL_ENABLE_BIT);
		#endif
		glEnable(GL_LINE_SMOOTH);
		ofSetLineWidth(lineWidth);
		glHint (GL_LINE_SMOOTH_HINT, GL_FASTEST);
        
        for (int j=0; j<values.size(); j++) {
            
            Value & v = values[j];
            
            if(needsMesh){
                refillPlotMesh(v.plotMesh, v.values, x, y, w, h);
                if (showSmoothedPlot){
                    refillPlotMesh(v.smoothPlotMesh, v.smoothValues, x, y, w, h);
                }
            }
            
            if (colorSet){
#ifndef TARGET_OPENGLES
                glPushAttrib(GL_CURRENT_BIT);
#endif
                if(showSmoothedPlot){
                    glColor4ub(v.lineColor.r, v.lineColor.g, v.lineColor.b, v.lineColor.a / 3);
                }else{
                    glColor4ub(v.lineColor.r, v.lineColor.g, v.lineColor.b, v.lineColor.a);
                }
            }
            v.plotMesh.draw();
            if (showSmoothedPlot){
                if (colorSet){
                    glColor4ub(v.lineColor.r, v.lineColor.g, v.lineColor.b, v.lineColor.a);
                }
                
                v.smoothPlotMesh.draw();
            }
        }

		#ifndef TARGET_OPENGLES
			if (colorSet) glPopAttrib();
		glPopAttrib();
		#endif

		ofFill();
	}
	prevRect = r;
}


void ofxHistoryPlot::setRange(float low, float high){

	manualRange = true;
	onlyLowestIsFixed = false;
	lowest = low;
	highest = high;
}

float ofxHistoryPlot::getLowerRange(){
	return lowest;
}


float ofxHistoryPlot::getHigerRange(){
	return highest;
}


void ofxHistoryPlot::setLowerRange(float low){
	
	manualRange = true;
	onlyLowestIsFixed = true;
	lowest = low;
}