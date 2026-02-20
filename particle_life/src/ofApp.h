#pragma once

#include "ofMain.h"
#include "ofxGui.h"
//#include <sdlt/sdlt.h>


// use a SOA that represents a group of particles with the same color
struct colorGroup {
	std::vector<ofVec2f> pos;
	std::vector<ofVec2f> vel;
	ofColor color;
};


class ofApp final : public ofBaseApp
{
public:
	void setup() override;
	void update() override;
	void draw() override;
	void keyPressed(int key) override;
	void restart();
	void random();
	void saveSettings();
	void loadSettings();
	void interaction(colorGroup& Group1, const colorGroup& Group2, const float G, const float radius, bool boundsToggle) const noexcept;

	static float RandomFloat(const float a, const float b) { return a + (ofRandomuf() * (b - a)); }

	void Draw(colorGroup group)
	{
		ofSetColor(group.color);
		vbo.updateVertexData(group.pos.data(), group.pos.size());
		vbo.draw(GL_POINTS, 0, group.pos.size());

	}
	
	ofxPanel gui;
	ofVbo vbo;

	//TODO change these colors to match web version
	colorGroup green;
	colorGroup red;
	colorGroup orange;
	colorGroup cyan;

	int cntFps = 0;
	clock_t now, lastTime, delta;
	clock_t lastTime_draw, delta_draw;
	clock_t physic_begin, physic_delta;

#pragma region guigroup
	ofxGuiGroup globalGroup;
	ofxGuiGroup qtyGroup;
	ofxGuiGroup redGroup;
	ofxGuiGroup greenGroup;
	ofxGuiGroup cyanGroup;
	ofxGuiGroup ornageGroup;
#pragma endregion guigroup

	ofxButton resetButton;
	ofxButton selectButton;
	ofxButton randomChoice;
	ofxButton save;
	ofxButton load;

	ofxToggle boundsToggle;
	ofxToggle modelToggle;

#pragma region some experimental features
	ofxGuiGroup expGroup;
	ofxToggle evoToggle;
	ofxFloatSlider evoProbSlider;
	ofxFloatSlider evoAmountSlider;
	float evoChance = 1;
	float evoAmount = 1;
	ofxToggle radiusToogle;
	ofxLabel physicLabel;
	ofxIntSlider probabilitySlider;
	unsigned int probability = 100;
	ofxToggle motionBlurToggle;
#pragma endregion some experimental features



#pragma region slider
	ofxIntSlider numberSliderR;
	ofxIntSlider numberSliderG;
	ofxIntSlider numberSliderO;
	ofxIntSlider numberSliderC;

	ofxFloatSlider viscoSlider;
	ofxFloatSlider gravitySlider;
	ofxFloatSlider wallRepelSlider;

	ofxFloatSlider powerSliderRR;
	ofxFloatSlider powerSliderRG;
	ofxFloatSlider powerSliderRO;
	ofxFloatSlider powerSliderRC;
	
	ofxFloatSlider powerSliderGR;
	ofxFloatSlider powerSliderGG;
	ofxFloatSlider powerSliderGO;
	ofxFloatSlider powerSliderGC;
	
	ofxFloatSlider powerSliderOR;
	ofxFloatSlider powerSliderOG;
	ofxFloatSlider powerSliderOO;
	ofxFloatSlider powerSliderOC;
	
	ofxFloatSlider powerSliderCR;
	ofxFloatSlider powerSliderCG;
	ofxFloatSlider powerSliderCO;
	ofxFloatSlider powerSliderCC;
	
	ofxFloatSlider vSliderRR;
	ofxFloatSlider vSliderRG;
	ofxFloatSlider vSliderRO;
	ofxFloatSlider vSliderRC;
	
	ofxFloatSlider vSliderGR;
	ofxFloatSlider vSliderGG;
	ofxFloatSlider vSliderGO;
	ofxFloatSlider vSliderGC;
	
	ofxFloatSlider vSliderOR;
	ofxFloatSlider vSliderOG;
	ofxFloatSlider vSliderOO;
	ofxFloatSlider vSliderOC;

	ofxFloatSlider vSliderCR;
	ofxFloatSlider vSliderCG;
	ofxFloatSlider vSliderCO;
	ofxFloatSlider vSliderCC;

	vector<ofxFloatSlider*> powersliders = {
		&powerSliderRR, &powerSliderRG, &powerSliderRC, &powerSliderRO,
		&powerSliderGR, &powerSliderGG, &powerSliderGC, &powerSliderGO,
		&powerSliderCR, &powerSliderCG, &powerSliderCC, &powerSliderCO,
		&powerSliderOR, &powerSliderOG, &powerSliderOC, &powerSliderOO,
	};

	vector<ofxFloatSlider*> vsliders = {
		&vSliderRR, &vSliderRG, &vSliderRC, &vSliderRO,
		&vSliderGR, &vSliderGG, &vSliderGC, &vSliderGO,
		&vSliderCR, &vSliderCG, &vSliderCC, &vSliderCO,
		&vSliderOR, &vSliderOG, &vSliderOC, &vSliderOO,
	};

#pragma endregion slider

#pragma region slider values
	unsigned int pnumberSliderR = 1000;
	unsigned int pnumberSliderG = 1000;
	unsigned int pnumberSliderO = 1000;
	unsigned int pnumberSliderC = 1000;

	float ppowerSliderRR = 0;
	float ppowerSliderRG = 0;
	float ppowerSliderRO = 0;
	float ppowerSliderRC = 0;
	
	float ppowerSliderGR = 0;
	float ppowerSliderGG = 0;
	float ppowerSliderGO = 0;
	float ppowerSliderGC = 0;

	float ppowerSliderOR = 0;
	float ppowerSliderOG = 0;
	float ppowerSliderOO = 0;
	float ppowerSliderOC = 0;

	float ppowerSliderCR = 0;
	float ppowerSliderCG = 0;
	float ppowerSliderCO = 0;
	float ppowerSliderCC = 0;

	float pvSliderRR = 180;
	float pvSliderRG = 180;
	float pvSliderRO = 180;
	float pvSliderRC = 180;

	float pvSliderGR = 180;
	float pvSliderGG = 180;
	float pvSliderGO = 180;
	float pvSliderGC = 180;

	float pvSliderOR = 180;
	float pvSliderOG = 180;
	float pvSliderOO = 180;
	float pvSliderOC = 180;

	float pvSliderCR = 180;
	float pvSliderCG = 180;
	float pvSliderCO = 180;
	float pvSliderCC = 180;

#pragma endregion slider values

	ofxLabel labelG;
	ofxLabel labelR;
	ofxLabel labelW;
	ofxLabel labelY;
	
	ofxLabel aboutL1;
	ofxLabel aboutL2;
	ofxLabel aboutL3;
	ofxLabel fps;

	std::string fps_text;
	std::string physic_text;


	// simulation bounds
	unsigned int boundWidth = 1600;
	unsigned int boundHeight = 900;

	float viscosity = 0.5F;
	float worldGravity = 0.0F;
	float forceVariance = 0.8F;
	float radiusVariance = 0.6F;
	float wallRepel = 10.0F;
};
