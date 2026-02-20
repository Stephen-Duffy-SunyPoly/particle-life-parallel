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

constexpr size_t GREEN_INDEX = 0;
constexpr size_t RED_INDEX = 1;
constexpr size_t ORANGE_INDEX = 2;
constexpr size_t CYAN_INDEX = 3;

/**Hold sliders to represent a property for each color that can be accessed in a dynamic way at runtime
 */
struct PerColorSliderValues {
	ofxFloatSlider red ;
	ofxFloatSlider green ;
	ofxFloatSlider orange;
	ofxFloatSlider cyan ;

	ofxFloatSlider& operator[](size_t index){
		switch (index) {
			case GREEN_INDEX:
				return green;
			case RED_INDEX:
				return red;
			case ORANGE_INDEX:
				return orange;
			case CYAN_INDEX:
				return cyan;
			default:
				return green;
		}
	};
};

/**Hold collections of sliders for each color on color property for each color that can be accessed in a dynamic way at runtime
 */
struct GroupColorSliderValues {
	PerColorSliderValues red;
	PerColorSliderValues green;
	PerColorSliderValues orange;
	PerColorSliderValues cyan;

	PerColorSliderValues& operator[](size_t index) {
		switch (index) {
			case GREEN_INDEX:
				return green;
			case RED_INDEX:
				return red;
			case ORANGE_INDEX:
				return orange;
			case CYAN_INDEX:
				return cyan;
			default:
				return green;
		}
	}
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

	GroupColorSliderValues colorPowerSliders = {};
	GroupColorSliderValues colorRadiusSliders = {};

	vector<ofxFloatSlider*> powersliders = {
		&colorPowerSliders.red.red,    &colorPowerSliders.red.green,    &colorPowerSliders.red.cyan,    &colorPowerSliders.red.orange,
		&colorPowerSliders.green.red,  &colorPowerSliders.green.green,  &colorPowerSliders.green.cyan,  &colorPowerSliders.green.orange,
		&colorPowerSliders.cyan.red,   &colorPowerSliders.cyan.green,   &colorPowerSliders.cyan.cyan,   &colorPowerSliders.cyan.orange,
		&colorPowerSliders.orange.red, &colorPowerSliders.orange.green, &colorPowerSliders.orange.cyan, &colorPowerSliders.orange.orange
	};

	vector<ofxFloatSlider*> vsliders = {
		&colorRadiusSliders.red.red,    &colorRadiusSliders.red.green,    &colorRadiusSliders.red.cyan,    &colorRadiusSliders.red.orange,
		&colorRadiusSliders.green.red,  &colorRadiusSliders.green.green,  &colorRadiusSliders.green.cyan,  &colorRadiusSliders.green.orange,
		&colorRadiusSliders.cyan.red,   &colorRadiusSliders.cyan.green,   &colorRadiusSliders.cyan.cyan,   &colorRadiusSliders.cyan.orange,
		&colorRadiusSliders.orange.red, &colorRadiusSliders.orange.green, &colorRadiusSliders.orange.cyan, &colorRadiusSliders.orange.orange
	};

#pragma endregion slider



#pragma region slider values
	unsigned int pnumberSliderR = 1000;
	unsigned int pnumberSliderG = 1000;
	unsigned int pnumberSliderO = 1000;
	unsigned int pnumberSliderC = 1000;

	float defaultPowerValue = 0;
	float defaultRadiusValue = 180;

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
