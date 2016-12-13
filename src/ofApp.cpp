#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  // MARK: - Initialise socket connection
  isConnected = false;
  address = "http://localhost:3000";
  status = "not connected";
  socketIO.setup(address);
  ofAddListener(socketIO.notifyEvent, this, &ofApp::gotEvent);
  ofAddListener(socketIO.connectionEvent, this, &ofApp::onConnection);
  hoopX = 0; hoopY = 0; hoopScale = 0;
  
  // MARK: - Initialise Kinect connection
  kinect.init();
  kinect.open();
  if (kinect.isConnected()) {
    easyCam.enableMouseInput();
    pointCloud.setMode(OF_PRIMITIVE_POINTS);
  }
  
  // MARK: - Preset drawing setting
  ofSetWindowShape(640, 480);
  ofSetCircleResolution(64);
  ofSetFrameRate(60);
  pointsDetected = 0;
  
  // MARK: - Initialise debug interface
  gui.setup();
  gui.add(beamerDistanceSlider.setup("Beamer distance", 0.5, 0, 1.0));
	gui.add(kinectXSlider.setup("Kinect Scale X", 1.6, -2, 2));
	gui.add(kinectYSlider.setup("Kinect Scale Y", -1.6, -2, 2));
	gui.add(kinectZSlider.setup("Kinect Scale Z", -1, -2, 2));
	gui.add(kinectAngleSlider.setup("Kinect Angle", 0, -30, 30));
	gui.add(kinectSphereZSlider.setup("Sphere Z", 0, -500, 2000));
}

void ofApp::update() {
  // MARK: - Generate pointcloud from Kinect data
  pointsDetected = 0;
  if(kinect.isConnected()) {
    kinect.update();
    if(kinect.isFrameNewDepth()) {
      pointCloud.clear();
      for(int y = 0; y < kinect.height; y++) {
        for(int x= 0; x < kinect.width; x++) {
          int z = kinect.getDistanceAt(x, y);
          if(z > 0) {
            pointCloud.addColor(kinect.getColorAt(x, y));
            ofVec3f pt = kinect.getWorldCoordinateAt(x, y);
            pointCloud.addVertex(pt);
          }
          if(x > hoopX - (hoopScale*100)/2 && x < hoopX + (hoopScale*100)/2) {
            if(y > hoopY - (hoopScale*100)/2 && y < hoopY + (hoopScale*100)/2) {
              if(z > kinectSphereZSlider - (hoopScale*100)/2 && z < kinectSphereZSlider + (hoopScale*100)/2) {
                pointsDetected++;
              }
            }
          }
        }
      }
    }
  }
}

void ofApp::draw() {
  ofBackground(0);
  
  // MARK: - Draw hoop if global vars are not null
  if(hoopX && hoopY && hoopScale) {
    ofSetColor(255, 105, 180);
    ofNoFill();
    ofSetLineWidth(5);
    ofDrawCircle(hoopX, hoopY, (hoopScale * 100));
  }
  
  // MARK: - Display the pointcloud & start easyCam
  drawPointCloud();
  
  // MARK: - Draw debug interface
  if(debugMode) {
    gui.draw();
    kinect.setCameraTiltAngle(kinectAngleSlider);
  }
}

// MARK: - #BIND_ON_CONNECT
void ofApp::onConnection () {
  isConnected = true;
  bindEvents();
}

// MARK: - #BIND_EVENTS_TO_CALLBACKS
void ofApp::bindEvents () {
  std::string hoopPlacedEventName = "drawHoop";
  socketIO.bindEvent(hoopPlacedEvent, hoopPlacedEventName);
  ofAddListener(hoopPlacedEvent, this, &ofApp::drawHoop);
}

// MARK: - #PROPAGATE_EVENT
void ofApp::gotEvent(string& name) {
  ofLogNotice("ofxSocketIO[gotEvent]", name);
  status = name;
}

// MARK: - #DRAWS
void ofApp::drawPointCloud() {
  easyCam.begin();
  glPushMatrix();
	debugMode ? ofScale(kinectXSlider, kinectYSlider, kinectZSlider) : ofScale(1.6, -1.6, -1);
  ofTranslate(0, 0, 0);
	ofPushMatrix();
  ofEnableDepthTest();
  pointCloud.drawVertices();
  ofNoFill();
  ofDisableDepthTest();
  glPopMatrix();
	ofTranslate(-1024/2, -768/2, 0);
  if(pointsDetected > 100) ofLog() << "JA EINDELIJK";
	if (hoopX && hoopY && hoopScale) ofDrawSphere(hoopX, hoopY, kinectSphereZSlider, hoopScale * 100);
	ofPopMatrix();
  easyCam.end();
}

// MARK: - #EVENTS
void ofApp::drawHoop (ofxSocketIOData& data) {
  // MARK: - Log data to console
  ofLogNotice("ofxSocketIO[pageX]", ofToString(data.getIntValue("pageX")));
  ofLogNotice("ofxSocketIO[pageY]", ofToString(data.getIntValue("pageY")));
  ofLogNotice("ofxSocketIO[scale]", ofToString(data.getFloatValue("scale")));
  
  // MARK: - Assign data values to global variables
  hoopX = data.getIntValue("pageX");
  hoopY = data.getIntValue("pageY");
  hoopScale = data.getFloatValue("scale");
}
