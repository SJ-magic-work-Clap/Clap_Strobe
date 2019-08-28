/************************************************************
************************************************************/
#include "ofApp.h"

/************************************************************
************************************************************/
const char targetIP[] = "10.7.206.7";


/************************************************************
************************************************************/

/******************************
******************************/
ofApp::ofApp()
: Osc_ClapDetector("127.0.0.1", 12348, 12347)
, Osc_video("127.0.0.1", 12352, 12351)
, b_ClapMessage(false)
, b_FeverMessage(false)
, b_FeverStopMessage(false)
, State(STATE__WAIT_CLAP)
, Duration(0.6)
, Duration_Fever(1.5)
{
}

/******************************
******************************/
ofApp::~ofApp()
{
	SendZero_Artnet();
}

/******************************
******************************/
void ofApp::exit(){
	printf("Good bye\n");
}

/******************************
******************************/
void ofApp::SetZero_Artnet(){
	for(int i = 0; i < DMX_SIZE; i++){
		data[i] = 0;
	}
}

/******************************
******************************/
void ofApp::SendZero_Artnet(){
	SetZero_Artnet();
	SendDmx();
}

//--------------------------------------------------------------
void ofApp::setup(){
	/********************
	********************/
	ofSetWindowTitle("Clap:Strobe");
	ofSetVerticalSync(true);
	ofSetFrameRate(60);
	ofSetWindowShape(WIDTH, HEIGHT);
	ofSetEscapeQuitsApp(false);
	
	ofEnableAlphaBlending();
	// ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	
	ofEnableSmoothing();
	
	/********************
	********************/
    artnet.setup(targetIP);
	artnet.enableThread(60.0);	// main windowと同じにしておくのが安全かな.
	SetZero_Artnet();
	
	/********************
	********************/
	gui.setup();
	{
		ofColor initColor = ofColor(255, 255, 255, 255);
		ofColor minColor = ofColor(0, 0, 0, 0);
		ofColor maxColor = ofColor(255, 255, 255, 255);
		gui.add(StrobeColor.setup("color", initColor, minColor, maxColor));
	}
}

//--------------------------------------------------------------
void ofApp::update(){
	/********************
	********************/
	while(Osc_ClapDetector.OscReceive.hasWaitingMessages()){
		ofxOscMessage m_receive;
		Osc_ClapDetector.OscReceive.getNextMessage(&m_receive);
		
		if(m_receive.getAddress() == "/DetectClap"){
			m_receive.getArgAsInt32(0); // 読み捨て
			
			b_ClapMessage = true;
		}
	}
	
	while(Osc_video.OscReceive.hasWaitingMessages()){
		ofxOscMessage m_receive;
		Osc_video.OscReceive.getNextMessage(&m_receive);
		
		if(m_receive.getAddress() == "/StopFever"){
			m_receive.getArgAsInt32(0); // 読み捨て
			// b_FeverStopMessage = true;
		}else if(m_receive.getAddress() == "/Fever"){
			m_receive.getArgAsInt32(0); // 読み捨て
			// b_FeverMessage = true;
		}
	}
	
	/********************
	********************/
	StateChart();
	
	/********************
	********************/
	b_ClapMessage		= false;
	b_FeverMessage		= false;
	b_FeverStopMessage	= false;
}

/******************************
******************************/
void ofApp::StateChart(){
	/********************
	********************/
	float now = ofGetElapsedTimef();
	
	/********************
	********************/
	switch(State){
		case STATE__WAIT_CLAP:
			if(b_FeverMessage){
				State = STATE__FEVER;
				t_FeverFrom = now;
			}else if(b_ClapMessage){
				State = STATE__STROBE;
				t_From = now;
			}
			break;
			
		case STATE__STROBE:
			if(b_FeverMessage){
				State = STATE__FEVER;
				t_FeverFrom = now;
			}else if(b_ClapMessage){
				t_From = now;
			}
			break;
			
		case STATE__FEVER:
			if((b_FeverStopMessage) || (Duration_Fever < now - t_FeverFrom)){
				State = STATE__WAIT_CLAP;
			}else if(b_FeverMessage){
				t_FeverFrom = now;
			}
			break;
	}
	
	/********************
	********************/
	switch(State){
		case STATE__WAIT_CLAP:
			SetZero_Artnet();
			break;
			
		case STATE__STROBE:
			Progress = (now - t_From) * 100 / Duration;
			if(100 < Progress) Progress = 100;
			
			if(100 <= Progress){
				State = STATE__WAIT_CLAP;
			}
			
			calColor_setDataArray();
			break;
			
		case STATE__FEVER:
			Progress = (now - t_FeverFrom) * 100 / Duration_Fever;
			if(100 < Progress) Progress = 100;
			
			Fever_setDataArray();
			break;
	}
}

/******************************
return
	0 - 1.
******************************/
double ofApp::calLev_Strobe(double Progress){
	if(Progress < 0){
		return 0;
	}else if(100 < Progress){
		return 0;
	}else{
		double tan = -double(1)/100;
		return 1 + tan * Progress;
	}
}

/******************************
******************************/
void ofApp::calColor_setDataArray(){
	/********************
	********************/
	ofColor maxColor = StrobeColor;
	double Lev = calLev_Strobe(Progress);
	
	/********************
	artnetの配列格納は、Ledのch仕様によって異なる.
	********************/
#if (LEDTYPE == LEDTYPE_TAPE)
	const int NUM_ITEMS_IN_CH = 3;
	
	for(int id = 0; id < 4; id++){
		data[NUM_ITEMS_IN_CH * id + 0] = int(maxColor.r * Lev);
		data[NUM_ITEMS_IN_CH * id + 1] = int(maxColor.g * Lev);
		data[NUM_ITEMS_IN_CH * id + 2] = int(maxColor.b * Lev);
	}
	
#else	
	
	const int NUM_ITEMS_IN_CH = 6;
	int id = 0;
	
	data[NUM_ITEMS_IN_CH * id + 0] = maxColor.a;
	data[NUM_ITEMS_IN_CH * id + 1] = int(maxColor.r * Lev);
	data[NUM_ITEMS_IN_CH * id + 2] = int(maxColor.g * Lev);
	data[NUM_ITEMS_IN_CH * id + 3] = int(maxColor.b * Lev);
	
	data[NUM_ITEMS_IN_CH * id + 4] = 0; // W
	data[NUM_ITEMS_IN_CH * id + 5] = 1; // Strobe. 1-9:open
	
#endif	
	/********************
	********************/
	/*
	printf("%5.2f, (%5d, %5d, %5d)\r", Lev, data[NUM_ITEMS_IN_CH * id + 1], data[NUM_ITEMS_IN_CH * id + 2], data[NUM_ITEMS_IN_CH * id + 3]);
	fflush(stdout);
	*/
}

/******************************
description
	Ledごと、仕様異なる
******************************/
double ofApp::calSpeed_Strobe(double Progress){
	enum{
		SPEED_MAX = 132,
		
		// SPEED_MIN = 10,
		SPEED_MIN = 100,
	};
	
	if(Progress < 0){
		return SPEED_MAX;
	}else if(100 < Progress){
		return SPEED_MIN;
	}else{
		double tan = -double(SPEED_MAX - SPEED_MIN)/100;
		return SPEED_MAX + tan * Progress;
	}
}

/******************************
******************************/
void ofApp::Fever_setDataArray(){
	/********************
	********************/
	ofColor maxColor = StrobeColor;
	double StrobeSpeed = calSpeed_Strobe(Progress);
	
	/********************
	artnetの配列格納は、Ledのch仕様によって異なる.
	********************/
	const int NUM_ITEMS_IN_CH = 6;
	int id = 0;
	
	data[NUM_ITEMS_IN_CH * id + 0] = maxColor.a;
	data[NUM_ITEMS_IN_CH * id + 1] = int(maxColor.r);
	data[NUM_ITEMS_IN_CH * id + 2] = int(maxColor.g);
	data[NUM_ITEMS_IN_CH * id + 3] = int(maxColor.b);
	
	data[NUM_ITEMS_IN_CH * id + 4] = 0; // W
	data[NUM_ITEMS_IN_CH * id + 5] = StrobeSpeed; // Strobe. 1-9:open, 10-132:Slow-Fast, 133-255:random
	
	/********************
	********************/
	/*
	printf("%5.2f, (%5d, %5d, %5d)\r", Lev, data[NUM_ITEMS_IN_CH * id + 1], data[NUM_ITEMS_IN_CH * id + 2], data[NUM_ITEMS_IN_CH * id + 3]);
	fflush(stdout);
	*/
}

/******************************
******************************/
void ofApp::SendDmx(){
	ofxArtnetMessage message;
	message.setData(data, DMX_SIZE);
	artnet.sendArtnet(message);
}

//--------------------------------------------------------------
void ofApp::draw(){
	/********************
	********************/
	ofBackground(30);
	
	/********************
	********************/
	SendDmx();
	
	/********************
	********************/
	gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch(key){
		case ' ':
			b_ClapMessage = true;
			break;
			
		case 'f':
			b_FeverMessage = true;
			break;
			
		case 's':
			b_FeverStopMessage = true;
			break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
