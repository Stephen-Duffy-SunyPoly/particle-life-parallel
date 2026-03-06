#include "ofApp.h"
#include "ofUtils.h"

#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <thread> //c++ standard lib threading API
#include <mutex> //c++ standard lib mutex API


struct ComputeThreadInfo {
	std::thread thread;
	std::mutex startSync;
	std::mutex endSync;
	int colorIndex =0;
	int parentColorIndex=0;
	vector<ofVec2f> velocities;
	bool shutdown = false;
};

struct ManagementThreadInfo {
	ComputeThreadInfo computeThreads[3];
	std::thread thread;
	std::mutex startSync;
	std::mutex endSync;
	int colorIndex;
	vector<ofVec2f> &combinedVelocities;//direct ref to the velocity's in the color group
	bool shutdown;
};


//Simulation parameters

// Create a group of particles with the same color and return it in a vector
colorGroup CreatePoints(const int num, ofColor color) noexcept
{
	colorGroup points;
	points.pos.reserve(num);
	points.vel.reserve(num);
	points.color = color;

	for (auto i = 0; i < num; i++)
	{
		int x = static_cast<int>(ofRandomWidth());
		int y = static_cast<int>(ofRandomHeight());	
		points.pos.emplace_back(x,y);
		points.vel.emplace_back(0.0F, 0.0F);
	}
		
	return points;
}

// compute and apply the interaction between two groups of particles
void ofApp::interaction(int colorGroup1Index,int colorGroup2Index, vector<ofVec2f> &velocityOut) noexcept
{

	float G = colorPowerSliders[colorGroup1Index][colorGroup2Index];
	float radius = colorRadiusSliders[colorGroup1Index][colorGroup2Index];

	colorGroup &Group1 = colorGroups[colorGroup1Index];
	colorGroup &Group2 = colorGroups[colorGroup2Index];
	
	assert(Group1.pos.size() % 64 == 0);
	assert(Group2.pos.size() % 64 == 0);
	
	const float g = G / -100;	// attraction coefficient

	// for each object in this group
	for (size_t i = 0; i < Group1.pos.size(); i++) {
		velocityOut[i] = {0,0};
		float fx = 0.0F;	// force on x
		float fy = 0.0F;	// force on y
		
		
		for (size_t j = 0; j < Group2.pos.size(); j++) //for each object in the other group
		{
			//store the positions for both this and the other particle in a local var, as well as have the velocity total as a local var as well
			if (Group1.pos[i] != Group2.pos[j])
			{
				//direct multiplication is more efficient then pow
				//			         this is inefficient, why compute that difference twice when we could compute it once
				const float distance_squared = ((Group1.pos[i].x - Group2.pos[j].x) * (Group1.pos[i].x - Group2.pos[j].x))
											 + ((Group1.pos[i].y - Group2.pos[j].y) * (Group1.pos[i].y - Group2.pos[j].y));
				// pre compute radius squared outside of loop
				const float force = distance_squared < radius * radius ? 1.0F / std::sqrtf(distance_squared) : 0.0F;
				//store these values to a thread local cache
				//re-use pre-computed diffs here
				fx += ((Group1.pos[i].x - Group2.pos[j].x) * force);
				fy += ((Group1.pos[i].y - Group2.pos[j].y) * force);
				//on the manager thread reduce these values together from all the compyte threads
			}
		}

		//do this on main thread after computing new positions
		// Wall Repel
		if (wallRepel > 0.0F)
		{
			velocityOut[i].x += Group1.pos[i].x < wallRepel ? (wallRepel - Group1.pos[i].x) * 0.1 : 0.0F;	// x
			velocityOut[i].y += Group1.pos[i].y < wallRepel ? (wallRepel - Group1.pos[i].y) * 0.1 : 0.0F;	// x
			velocityOut[i].x += Group1.pos[i].x > boundWidth - wallRepel  ? (boundWidth - wallRepel - Group1.pos[i].x) * 0.1  : 0.0F; // y
			velocityOut[i].y += Group1.pos[i].y > boundHeight - wallRepel ? (boundHeight - wallRepel - Group1.pos[i].y) * 0.1 : 0.0F; // y
		}

		//do this on main thread after the parallel section finishes
		// Viscosity & gravity
		//perhaps store this computation of viscosity instead of what is currently stored
		//sture the computed velocities locally and reduce them together after each thread for a specific color finishes its job
		velocityOut[i].x = (velocityOut[i].x + (fx * g)) * (1.0 - viscosity);
		velocityOut[i].y = (velocityOut[i].y + (fy * g)) * (1.0 - viscosity) + worldGravity;

		//Update position do this outsize of the loop, after each thread does its thing, loop through every point and update this value
		//TODO use the opperator overload for this
		// Group1.pos[i].x += Group1.vel[i].x;
		// Group1.pos[i].y += Group1.vel[i].y;
	}

	// if "bounded" is checked then keep particles inside the window
	//TODO this outside of the threads
	// if (boundsToggle) {
	// 	for (auto& p : Group1.pos)
	// 	{
	// 		p.x = std::min(std::max(p.x, 0.0F), static_cast<float>(boundWidth));
	// 		p.y = std::min(std::max(p.y, 0.0F), static_cast<float>(boundHeight));
	// 	}
	// }
	
}
	

/**
 * @brief Generate new sets of points
 */
void ofApp::restart()
{
	// Ensure that the number of particles is a multiple of 64 in order to use the vectorized version of the interaction function
	numberSliderG = numberSliderG - (numberSliderG % 64);
	numberSliderR = numberSliderR - (numberSliderR % 64);
	numberSliderO = numberSliderO - (numberSliderO % 64);
	numberSliderC = numberSliderC - (numberSliderC % 64);
	
	assert(numberSliderG % 64 == 0);
	assert(numberSliderR % 64 == 0);
	assert(numberSliderW % 64 == 0);
	assert(numberSliderY % 64 == 0);
	
	// Create the groups of particles
	if (numberSliderG > 0) { colorGroups[GREEN_INDEX]  = CreatePoints(numberSliderG, ofColor::green); }
	if (numberSliderR > 0) { colorGroups[RED_INDEX]    = CreatePoints(numberSliderR,   ofColor::red);   }
	if (numberSliderO > 0) { colorGroups[ORANGE_INDEX] = CreatePoints(numberSliderO, ofColor::orange); }
	if (numberSliderC > 0) { colorGroups[CYAN_INDEX]   = CreatePoints(numberSliderC,  ofColor::cyan); }

	vbo.setVertexData(colorGroups[GREEN_INDEX].pos.data(),  colorGroups[GREEN_INDEX].pos.size(),  GL_STREAM_DRAW);
	vbo.setVertexData(colorGroups[RED_INDEX].pos.data(),    colorGroups[RED_INDEX].pos.size(),    GL_STREAM_DRAW);
	vbo.setVertexData(colorGroups[ORANGE_INDEX].pos.data(),  colorGroups[ORANGE_INDEX].pos.size(),  GL_STREAM_DRAW);
	vbo.setVertexData(colorGroups[CYAN_INDEX].pos.data(), colorGroups[CYAN_INDEX].pos.size(), GL_STREAM_DRAW);
}


/**
 * @brief Generate initial simulation parameters
 */
void ofApp::random()
{
	// GREEN
	//numberSliderG = RandomFloat(0, 3000);
	colorPowerSliders[GREEN_INDEX].green  = RandomFloat(-100, 100) * forceVariance;
	colorPowerSliders[GREEN_INDEX].red    = RandomFloat(-100, 100) * forceVariance;
	colorPowerSliders[GREEN_INDEX].orange = RandomFloat(-100, 100) * forceVariance;
	colorPowerSliders[GREEN_INDEX].cyan   = RandomFloat(-100, 100) * forceVariance;

	colorRadiusSliders[GREEN_INDEX].green  = RandomFloat(10, 200) * radiusVariance;
	colorRadiusSliders[GREEN_INDEX].red    = RandomFloat(10, 200) * radiusVariance;
	colorRadiusSliders[GREEN_INDEX].orange = RandomFloat(10, 200) * radiusVariance;
	colorRadiusSliders[GREEN_INDEX].cyan   = RandomFloat(10, 200) * radiusVariance;

	// RED
	//numberSliderR = RandomFloat(0, 3000);
	colorPowerSliders[RED_INDEX].green  = RandomFloat(-100, 100) * forceVariance;
	colorPowerSliders[RED_INDEX].red    = RandomFloat(-100, 100) * forceVariance;
	colorPowerSliders[RED_INDEX].orange = RandomFloat(-100, 100) * forceVariance;
	colorPowerSliders[RED_INDEX].cyan   = RandomFloat(-100, 100) * forceVariance;

	colorRadiusSliders[RED_INDEX].green  = RandomFloat(10, 200) * radiusVariance;
	colorRadiusSliders[RED_INDEX].red    = RandomFloat(10, 200) * radiusVariance;
	colorRadiusSliders[RED_INDEX].orange = RandomFloat(10, 200) * radiusVariance;
	colorRadiusSliders[RED_INDEX].cyan   = RandomFloat(10, 200) * radiusVariance;

	// orange
	// numberSliderW = RandomFloat(0, 3000);
	colorPowerSliders[ORANGE_INDEX].green  = RandomFloat(-100, 100) * forceVariance;
	colorPowerSliders[ORANGE_INDEX].red    = RandomFloat(-100, 100) * forceVariance;
	colorPowerSliders[ORANGE_INDEX].orange = RandomFloat(-100, 100) * forceVariance;
	colorPowerSliders[ORANGE_INDEX].cyan   = RandomFloat(-100, 100) * forceVariance;

	colorRadiusSliders[ORANGE_INDEX].green  = RandomFloat(10, 200) * radiusVariance;
	colorRadiusSliders[ORANGE_INDEX].red    = RandomFloat(10, 200) * radiusVariance;
	colorRadiusSliders[ORANGE_INDEX].orange = RandomFloat(10, 200) * radiusVariance;
	colorRadiusSliders[ORANGE_INDEX].cyan   = RandomFloat(10, 200) * radiusVariance;

	// cyan
	//numberSliderY = RandomFloat(0, 3000);
	colorPowerSliders[CYAN_INDEX].green  = RandomFloat(-100, 100) * forceVariance;
	colorPowerSliders[CYAN_INDEX].red    = RandomFloat(-100, 100) * forceVariance;
	colorPowerSliders[CYAN_INDEX].orange = RandomFloat(-100, 100) * forceVariance;
	colorPowerSliders[CYAN_INDEX].cyan   = RandomFloat(-100, 100) * forceVariance;

	colorRadiusSliders[CYAN_INDEX].green  = RandomFloat(10, 200) * radiusVariance;
	colorRadiusSliders[CYAN_INDEX].red    = RandomFloat(10, 200) * radiusVariance;
	colorRadiusSliders[CYAN_INDEX].orange = RandomFloat(10, 200) * radiusVariance;
	colorRadiusSliders[CYAN_INDEX].cyan   = RandomFloat(10, 200) * radiusVariance;
}

/// this is a cheap and quick way to save and load parameters (openFramework have betters ways but requires some additional library setups) 
// Dialog gui tested on windows machine only. Not sure if it works on Mac or Linux too.
void ofApp::saveSettings()
{
	const std::vector<float> settings = {
		colorPowerSliders[GREEN_INDEX].green, colorPowerSliders[GREEN_INDEX].red, colorPowerSliders[GREEN_INDEX].orange, colorPowerSliders[GREEN_INDEX].cyan,
		colorRadiusSliders[GREEN_INDEX].green, colorRadiusSliders[GREEN_INDEX].red, colorRadiusSliders[GREEN_INDEX].orange, colorRadiusSliders[GREEN_INDEX].cyan,
		colorPowerSliders[RED_INDEX].green, colorPowerSliders[RED_INDEX].red, colorPowerSliders[RED_INDEX].orange, colorPowerSliders[RED_INDEX].cyan ,
		colorRadiusSliders[RED_INDEX].green, colorRadiusSliders[RED_INDEX].red, colorRadiusSliders[RED_INDEX].orange, colorRadiusSliders[RED_INDEX].cyan,
		colorPowerSliders[ORANGE_INDEX].green, colorPowerSliders[ORANGE_INDEX].red, colorPowerSliders[ORANGE_INDEX].orange, colorPowerSliders[ORANGE_INDEX].cyan ,
		colorRadiusSliders[ORANGE_INDEX].green, colorRadiusSliders[ORANGE_INDEX].red, colorRadiusSliders[ORANGE_INDEX].orange, colorRadiusSliders[ORANGE_INDEX].cyan,
		colorPowerSliders[CYAN_INDEX].green, colorPowerSliders[CYAN_INDEX].red, colorPowerSliders[CYAN_INDEX].orange, colorPowerSliders[CYAN_INDEX].cyan,
		colorRadiusSliders[CYAN_INDEX].green, colorRadiusSliders[CYAN_INDEX].red, colorRadiusSliders[CYAN_INDEX].orange, colorRadiusSliders[CYAN_INDEX].cyan,
		static_cast<float>(numberSliderG),
		static_cast<float>(numberSliderR),
		static_cast<float>(numberSliderO),
		static_cast<float>(numberSliderC),
		viscoSlider
	};

	std::string save_path;
	ofFileDialogResult result = ofSystemSaveDialog("model.txt", "Save");
	if (result.bSuccess)
	{
		save_path = result.getPath();
	}
	else
	{
		ofSystemAlertDialog("Could not Save Model!");
	}
	std::ofstream myfile(save_path);
	if (myfile.is_open())
	{
		for (int i = 0; i < settings.size(); i++)
		{
			myfile << settings[i] << " ";
		}
		myfile.close();
		std::cout << "file saved successfully";
	}
	else
	{
		std::cout << "unable to save file!";
	}
}

// Dialog gui tested on windows machine only. Not sure if it works on Mac or Linux too.
void ofApp::loadSettings()
{
	std::string load_path;
	std::string text;
	ofFileDialogResult result = ofSystemLoadDialog("Load file", false, load_path);
	if (result.bSuccess)
	{
		load_path = result.getPath();
		std::ifstream t(load_path);
		std::stringstream loaded;
		loaded << t.rdbuf();
		text = loaded.str();
	}
	else
	{
		ofSystemAlertDialog("Could not Load the File!");
	}

	// split text by space and convert them to floats
	string word;
	std::vector<float> p;
	for (auto x : text)
	{
		if (x == ' ')
		{
			p.push_back(std::stof(word));
			word = "";
		}
		else
		{
			word += x;
		}
	}

	if (p.size() < 37)
	{
		// better checks needed
		ofSystemAlertDialog("Could not read the file!");
	}
	else
	{
		colorPowerSliders[GREEN_INDEX].green  = p[0];
		colorPowerSliders[GREEN_INDEX].red    = p[1];
		colorPowerSliders[GREEN_INDEX].orange = p[2];
		colorPowerSliders[GREEN_INDEX].cyan   = p[3];
		colorRadiusSliders[GREEN_INDEX].green = p[4];
		colorRadiusSliders[GREEN_INDEX].red    = p[5];
		colorRadiusSliders[GREEN_INDEX].orange = p[6];
		colorRadiusSliders[GREEN_INDEX].cyan   = p[7];
		colorPowerSliders[RED_INDEX].green  = p[8];
		colorPowerSliders[RED_INDEX].red    = p[9];
		colorPowerSliders[RED_INDEX].orange = p[10];
		colorPowerSliders[RED_INDEX].cyan   = p[11];
		colorRadiusSliders[RED_INDEX].green  = p[12];
		colorRadiusSliders[RED_INDEX].red    = p[13];
		colorRadiusSliders[RED_INDEX].orange = p[14];
		colorRadiusSliders[RED_INDEX].cyan   = p[15];
		colorPowerSliders[ORANGE_INDEX].green  = p[16];
		colorPowerSliders[ORANGE_INDEX].red    = p[17];
		colorPowerSliders[ORANGE_INDEX].orange = p[18];
		colorPowerSliders[ORANGE_INDEX].cyan   = p[19];
		colorRadiusSliders[ORANGE_INDEX].green  = p[20];
		colorRadiusSliders[ORANGE_INDEX].red    = p[21];
		colorRadiusSliders[ORANGE_INDEX].orange = p[22];
		colorRadiusSliders[ORANGE_INDEX].cyan   = p[23];
		colorPowerSliders[CYAN_INDEX].green  = p[24];
		colorPowerSliders[CYAN_INDEX].red    = p[25];
		colorPowerSliders[CYAN_INDEX].orange = p[26];
		colorPowerSliders[CYAN_INDEX].cyan   = p[27];
		colorRadiusSliders[CYAN_INDEX].green  = p[28];
		colorRadiusSliders[CYAN_INDEX].red    = p[29];
		colorRadiusSliders[CYAN_INDEX].orange = p[30];
		colorRadiusSliders[CYAN_INDEX].cyan   = p[31];
		numberSliderG = static_cast<int>(p[32]);
		numberSliderR = static_cast<int>(p[33]);
		numberSliderO = static_cast<int>(p[34]);
		numberSliderC = static_cast<int>(p[35]);
		viscoSlider = p[36];
	}
	probabilitySlider = 100;
	restart();
}


//------------------------------GUI initialization------------------------------
void ofApp::setup()
{
	
	lastTime = clock();
	lastTime_draw = lastTime;
	ofSetWindowTitle("Particle Life - www.brainxyz.com");
	ofSetVerticalSync(false);

	// Interface
	gui.setup("Settings");
	gui.loadFont("data/Arial.ttf", 12);
	gui.setWidthElements(300.0f);

	gui.add(fps.setup("FPS", "0"));

	gui.add(resetButton.setup("Restart"));
	gui.add(randomChoice.setup("Randomize (space bar)"));
	gui.add(save.setup("Save Model"));
	gui.add(load.setup("Load Model"));
	gui.add(modelToggle.setup("Show Model", false));

	globalGroup.setup("Global");
	globalGroup.add(viscoSlider.setup("Viscosity/Friction", viscosity, 0, 1));
	globalGroup.add(gravitySlider.setup("Gravity", worldGravity, -1, 1));
	globalGroup.add(wallRepelSlider.setup("Wall Repel", wallRepel, 0, 100));
	globalGroup.add(boundsToggle.setup("Bounded", false));

	gui.add(&globalGroup);

	// Quantity
	qtyGroup.setup("Quantity (require restart/randomize)");
	qtyGroup.add(numberSliderG.setup("Green", pnumberSliderG, 0, 10000));
	qtyGroup.add(numberSliderR.setup("Red", pnumberSliderR, 0, 10000));
	qtyGroup.add(numberSliderO.setup("Ornage", pnumberSliderO, 0, 10000));
	qtyGroup.add(numberSliderC.setup("Cyan", pnumberSliderC, 0, 10000));
	gui.add(&qtyGroup);

	// GREEN
	greenGroup.setup("Green");
	greenGroup.add(colorPowerSliders[GREEN_INDEX].green .setup("green x green:", defaultPowerValue, -100, 100));
	greenGroup.add(colorPowerSliders[GREEN_INDEX].red   .setup("green x red:", defaultPowerValue, -100, 100));
	greenGroup.add(colorPowerSliders[GREEN_INDEX].orange.setup("green x orange:", defaultPowerValue, -100, 100));
	greenGroup.add(colorPowerSliders[GREEN_INDEX].cyan  .setup("green x cyan:", defaultPowerValue, -100, 100));

	greenGroup.add(colorRadiusSliders[GREEN_INDEX].green .setup("radius g x g:", defaultRadiusValue, 10, 500));
	greenGroup.add(colorRadiusSliders[GREEN_INDEX].red   .setup("radius g x r:", defaultRadiusValue, 10, 500));
	greenGroup.add(colorRadiusSliders[GREEN_INDEX].orange.setup("radius g x o:", defaultRadiusValue, 10, 500));
	greenGroup.add(colorRadiusSliders[GREEN_INDEX].cyan  .setup("radius g x c:", defaultRadiusValue, 10, 500));

	greenGroup.minimize();
	gui.add(&greenGroup);

	// RED
	redGroup.setup("Red");
	redGroup.add(colorPowerSliders[RED_INDEX].red   .setup("red x red:", defaultPowerValue, -100, 100));
	redGroup.add(colorPowerSliders[RED_INDEX].green .setup("red x green:", defaultPowerValue, -100, 100));
	redGroup.add(colorPowerSliders[RED_INDEX].orange.setup("red x orange:", defaultPowerValue, -100, 100));
	redGroup.add(colorPowerSliders[RED_INDEX].cyan  .setup("red x cyan:", defaultPowerValue, -100, 100));

	redGroup.add(colorRadiusSliders[RED_INDEX].green .setup("radius r x g:", defaultRadiusValue, 10, 500));
	redGroup.add(colorRadiusSliders[RED_INDEX].red   .setup("radius r x r:", defaultRadiusValue, 10, 500));
	redGroup.add(colorRadiusSliders[RED_INDEX].orange.setup("radius r x o:", defaultRadiusValue, 10, 500));
	redGroup.add(colorRadiusSliders[RED_INDEX].cyan  .setup("radius r x c:", defaultRadiusValue, 10, 500));

	redGroup.minimize();
	gui.add(&redGroup);

	// WHITE
	ornageGroup.setup("Orange");
	ornageGroup.add(colorPowerSliders[ORANGE_INDEX].orange.setup("Orange x Orange:", defaultPowerValue, -100, 100));
	ornageGroup.add(colorPowerSliders[ORANGE_INDEX].red   .setup("Orange x red:", defaultPowerValue, -100, 100));
	ornageGroup.add(colorPowerSliders[ORANGE_INDEX].green .setup("Orange x green:", defaultPowerValue, -100, 100));
	ornageGroup.add(colorPowerSliders[ORANGE_INDEX].cyan  .setup("Orange x Cyan:", defaultPowerValue, -100, 100));

	ornageGroup.add(colorRadiusSliders[ORANGE_INDEX].green .setup("radius o x g:", defaultRadiusValue, 10, 500));
	ornageGroup.add(colorRadiusSliders[ORANGE_INDEX].red   .setup("radius o x r:", defaultRadiusValue, 10, 500));
	ornageGroup.add(colorRadiusSliders[ORANGE_INDEX].orange.setup("radius o x o:", defaultRadiusValue, 10, 500));
	ornageGroup.add(colorRadiusSliders[ORANGE_INDEX].cyan  .setup("radius o x c:", defaultRadiusValue, 10, 500));

	ornageGroup.minimize();
	gui.add(&ornageGroup);

	// yellow
	cyanGroup.setup("Cyan");
	cyanGroup.add(colorPowerSliders[CYAN_INDEX].cyan  .setup("Cyan x Cyan:", defaultPowerValue, -100, 100));
	cyanGroup.add(colorPowerSliders[CYAN_INDEX].orange.setup("Cyan x Orange:", defaultPowerValue, -100, 100));
	cyanGroup.add(colorPowerSliders[CYAN_INDEX].red   .setup("Cyan x red:", defaultPowerValue, -100, 100));
	cyanGroup.add(colorPowerSliders[CYAN_INDEX].green .setup("Cyan x green:", defaultPowerValue, -100, 100));

	cyanGroup.add(colorRadiusSliders[CYAN_INDEX].green .setup("radius c x g:", defaultRadiusValue, 10, 500));
	cyanGroup.add(colorRadiusSliders[CYAN_INDEX].red   .setup("radius c x r:", defaultRadiusValue, 10, 500));
	cyanGroup.add(colorRadiusSliders[CYAN_INDEX].orange.setup("radius c x o:", defaultRadiusValue, 10, 500));
	cyanGroup.add(colorRadiusSliders[CYAN_INDEX].cyan  .setup("radius c x c:", defaultRadiusValue, 10, 500));

	cyanGroup.minimize();
	gui.add(&cyanGroup);

	expGroup.setup("Experimental");
	expGroup.add(evoToggle.setup("Evolve parameters", false));
	expGroup.add(evoProbSlider.setup("evo chance%", evoChance, 0, 100));
	expGroup.add(evoAmountSlider.setup("evo amount%%", evoAmount, 0, 100));

	expGroup.add(motionBlurToggle.setup("Motion Blur", false));
	expGroup.add(physicLabel.setup("physic (ms)", "0"));

	expGroup.minimize();
	gui.add(&expGroup);

	ofSetBackgroundAuto(false);


	random();
	restart();
}

//------------------------------Update simulation with sliders values------------------------------
void ofApp::update()
{
	probability  = probabilitySlider;
	viscosity    = viscoSlider;
	worldGravity = gravitySlider;
	wallRepel    = wallRepelSlider;
	evoChance    = evoProbSlider;
	evoAmount    = evoAmountSlider;
	
	physic_begin = clock();
	boundHeight  = ofGetHeight();
	boundWidth   = ofGetWidth();

	if (evoToggle && ofRandom(1.0F) < (evoChance / 100.0F))
	{
		for (auto& slider : powersliders) {
			*slider = *slider + ((ofRandom(2.0F) - 1.0F) * (slider->getMax() - slider->getMin()) * (evoAmount / 100.0F));
		}

		for (auto& slider : vsliders) {
			*slider = *slider + ((ofRandom(2.0F) - 1.0F) * (slider->getMax() - slider->getMin()) * (evoAmount / 100.0F));
			if (*slider < 0.0F) *slider = 0;
			if (*slider > 200.0F) *slider = 200.0F;
		}
	}

		// interaction(red,   red,    colorPowerSliders[RED_INDEX].red,        colorRadiusSliders[RED_INDEX].red, boundsToggle);
		// interaction(red,   green,  colorPowerSliders[RED_INDEX].green,      colorRadiusSliders[RED_INDEX].green, boundsToggle);
		// interaction(red,   cyan,   colorPowerSliders[RED_INDEX].cyan,       colorRadiusSliders[RED_INDEX].cyan, boundsToggle);
		// interaction(red,   orange, colorPowerSliders[RED_INDEX].orange,     colorRadiusSliders[RED_INDEX].orange, boundsToggle);
		// interaction(green, red,    colorPowerSliders[GREEN_INDEX].red,      colorRadiusSliders[GREEN_INDEX].red, boundsToggle);
		// interaction(green, green,  colorPowerSliders[GREEN_INDEX].green,    colorRadiusSliders[GREEN_INDEX].green, boundsToggle);
		// interaction(green, cyan,   colorPowerSliders[GREEN_INDEX].cyan,     colorRadiusSliders[GREEN_INDEX].cyan, boundsToggle);
		// interaction(green, orange, colorPowerSliders[GREEN_INDEX].orange,   colorRadiusSliders[GREEN_INDEX].orange, boundsToggle);
		// interaction(cyan,  red,    colorPowerSliders[CYAN_INDEX].red,       colorRadiusSliders[CYAN_INDEX].red, boundsToggle);
		// interaction(cyan,  green,  colorPowerSliders[CYAN_INDEX].green,     colorRadiusSliders[CYAN_INDEX].green, boundsToggle);
		// interaction(cyan,  cyan,   colorPowerSliders[CYAN_INDEX].cyan,      colorRadiusSliders[CYAN_INDEX].cyan, boundsToggle);
		// interaction(cyan,  orange, colorPowerSliders[CYAN_INDEX].orange,    colorRadiusSliders[CYAN_INDEX].orange, boundsToggle);
		// interaction(orange, red,   colorPowerSliders[ORANGE_INDEX].red,     colorRadiusSliders[ORANGE_INDEX].red, boundsToggle);
		// interaction(orange, green, colorPowerSliders[ORANGE_INDEX].green,   colorRadiusSliders[ORANGE_INDEX].green, boundsToggle);
		// interaction(orange, cyan,  colorPowerSliders[ORANGE_INDEX].cyan,    colorRadiusSliders[ORANGE_INDEX].cyan, boundsToggle);
		// interaction(orange, orange, colorPowerSliders[ORANGE_INDEX].orange, colorRadiusSliders[ORANGE_INDEX].orange, boundsToggle);
		
	
	if (save) { saveSettings(); }
	if (load) { loadSettings(); }
	physic_delta = clock() - physic_begin;
}

//--------------------------------------------------------------
void ofApp::draw()
{
	//fps counter
	cntFps++;
	now = clock();
	delta = now - lastTime;
	delta_draw = now - lastTime_draw;

		if (motionBlurToggle)
		{
			ofSetColor(0, 0, 0, 64);
			ofDrawRectangle(0, 0, boundWidth, boundHeight);
		}
		else
		{
			ofClear(0);
		}

		//Time step
		if (delta >= 1000)
		{
			lastTime = now;
			fps.setup("FPS", to_string(static_cast<int>((1000 / static_cast<float>(delta)) * cntFps)));
			physicLabel.setup("physics (ms)", to_string(physic_delta));

			cntFps = 0;
		}

		//Check for GUI interaction
		if (resetButton) { restart(); }
		if (randomChoice)
		{
			random();
			restart();
		}
			
			if (numberSliderO > 0) { Draw(colorGroups[ORANGE_INDEX]); }
			if (numberSliderR > 0) { Draw(colorGroups[RED_INDEX]); }
			if (numberSliderG > 0) { Draw(colorGroups[GREEN_INDEX]); }
			if (numberSliderC > 0) { Draw(colorGroups[CYAN_INDEX]); }
			lastTime_draw = now;

			gui.draw();
}

void ofApp::keyPressed(int key)
{
	if(key == ' ')
	{
		random();
		restart();
	}
}


//unfortianly this needs to take a pointer
void computeThread(ComputeThreadInfo *info) {
	while (!info->shutdown) {
		//wait for the singal from the parent thread
		info->startSync.lock();
		if (info->shutdown) {
			return;
		}

		//do the compute

		//signal to the parent thread that this thread has finished
		info->endSync.unlock();
	}
}

void managementThread(ManagementThreadInfo * info) {
	for (int i=0;i<3;i++) {
		info->computeThreads[i].colorIndex=i+1;
		info->computeThreads[i].parentColorIndex = info->colorIndex;
		info->computeThreads[i].startSync.lock();
		info->computeThreads[i].endSync.lock();
		info->computeThreads[i].shutdown = false;
		info->computeThreads[i].velocities.resize(info->combinedVelocities.size());

		//start the compute thread
		info->computeThreads[i].thread = std::thread(computeThread,&info->computeThreads[i]);
	}

	while (!info->shutdown) {
		//wait for the signal from the main thread
		info->startSync.lock();

		if (info->shutdown) {
			break;
		}

		//signal all the sub threads
		for (auto & computeThread : info->computeThreads) {
			computeThread.startSync.unlock();
		}

		//do computations your self

		//wait for all the sub threads to finish
		for (auto & computeThread : info->computeThreads) {
			computeThread.endSync.lock();
		}

		//reduce the results of each thread
		for (size_t i=0;i<info->combinedVelocities.size();i++) {
			for (auto & computeThread : info->computeThreads) {
				info->combinedVelocities[i] += computeThread.velocities[i];
			}
		}

		//signal to the main thread ready
		info->endSync.unlock();
	}

	//tell all the child threads to shut down
	for (auto & computeThread : info->computeThreads) {
		computeThread.shutdown = true;//tell the thread to stop
		computeThread.startSync.unlock();//let it run
	}

	//wait for each thread to stop properly
	for (auto & computeThread : info->computeThreads) {
		computeThread.thread.join();
	}
	//this thread then stops
}