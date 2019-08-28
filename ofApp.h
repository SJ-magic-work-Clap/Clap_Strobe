/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#define LEDTYPE_PARTY	0
#define LEDTYPE_TAPE	1
	#define LEDTYPE LEDTYPE_PARTY

/************************************************************
************************************************************/
#include "ofMain.h"

#include "ofxArtnet.h"
#include "ofxOsc.h"
#include "ofxGui.h"

#include "sj_common.h"

/************************************************************
class
************************************************************/

/**************************************************
**************************************************/
class OSC_SEND{
private:
	char IP[BUF_SIZE];
	int Port;

	ofxOscSender sender;
	
public:
	OSC_SEND()
	: Port(-1)
	{
	}
	OSC_SEND(const char* _IP, int _Port)
	{
		if(_Port != -1){
			sprintf(IP, "%s", _IP);
			Port = _Port;
			
			sender.setup(IP, Port);
		}
	}
	
	void setup(const char* _IP, int _Port)
	{
		if(_Port != -1){
			sprintf(IP, "%s", _IP);
			Port = _Port;
			
			sender.setup(IP, Port);
		}
	}
	void sendMessage(ofxOscMessage &message)
	{
		if(Port != -1){
			sender.sendMessage(message);
		}
	}
};

/**************************************************
**************************************************/
class OSC_RECEIVE{
private:
	int Port;
	ofxOscReceiver	receiver;
	
public:
	OSC_RECEIVE()
	: Port(-1)
	{
	}
	OSC_RECEIVE(int _Port)
	{
		if(_Port != -1){
			Port = _Port;
			receiver.setup(Port);
		}
	}
	
	void setup(int _Port)
	{
		if(_Port != -1){
			Port = _Port;
			receiver.setup(Port);
		}
	}
	
	bool hasWaitingMessages()
	{
		if(Port == -1){
			return false;
		}else{
			return receiver.hasWaitingMessages();
		}
	}
	
	bool getNextMessage(ofxOscMessage *msg)
	{
		if(Port == -1){
			return false;
		}else{
			return receiver.getNextMessage(msg);
		}
	}
};

/**************************************************
**************************************************/
class OSC_TARGET{
private:
public:
	OSC_SEND	OscSend;
	OSC_RECEIVE	OscReceive;
	
	OSC_TARGET()
	{
	}
	
	OSC_TARGET(const char* _SendTo_IP, int _SendTo_Port, int _Receive_Port)
	: OscSend(_SendTo_IP, _SendTo_Port), OscReceive(_Receive_Port)
	{
	}
	
	void setup(const char* _SendTo_IP, int _SendTo_Port, int _Receive_Port)
	{
		OscSend.setup(_SendTo_IP, _SendTo_Port);
		OscReceive.setup(_Receive_Port);
	}
};


/**************************************************
**************************************************/
class ofApp : public ofBaseApp{
private:
	/****************************************
	****************************************/
	enum{
		WIDTH	= 300,
		HEIGHT	= 400,
	};
	enum{
		DMX_SIZE = 512,
	};
	
	enum STATE{
		STATE__WAIT_CLAP,
		STATE__STROBE,
		STATE__FEVER,
	};
	
	/****************************************
	****************************************/
	OSC_TARGET Osc_ClapDetector;
	OSC_TARGET Osc_video;
	
	bool b_ClapMessage;
	bool b_FeverMessage;
	bool b_FeverStopMessage;
	
	/********************
	********************/
	STATE State;
	
	const double Duration;
	float t_From;
	double Progress;
	
	float t_FeverFrom;
	const double Duration_Fever;
	
	/********************
	********************/
	unsigned char data[DMX_SIZE];
	ofxArtnetSender artnet;
	
	ofxPanel gui;
	ofxColorSlider StrobeColor;
	
	/****************************************
	****************************************/
	void calColor_setDataArray();
	void Fever_setDataArray();
	void SetZero_Artnet();
	void SendZero_Artnet();
	void StateChart();
	double calLev_Strobe(double Progress);
	double calSpeed_Strobe(double Progress);
	void SendDmx();
	
public:
	/****************************************
	****************************************/
	ofApp();
	~ofApp();
	
	void setup();
	void update();
	void draw();
	void exit();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
};
