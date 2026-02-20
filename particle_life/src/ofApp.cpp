#include "ofApp.h"
#include "ofUtils.h"

#include <iostream>
#include <vector>
#include <random>
#include <fstream>


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
void ofApp::interaction(colorGroup& Group1, const colorGroup& Group2, 
		const float G, const float radius, bool boundsToggle) const noexcept
{
	
	assert(Group1.pos.size() % 64 == 0);
	assert(Group2.pos.size() % 64 == 0);
	
	const float g = G / -100;	// attraction coefficient

	for (size_t i = 0; i < Group1.pos.size(); i++) // for each object in this group
	{
		float fx = 0.0F;	// force on x
		float fy = 0.0F;	// force on y
		
		
		for (size_t j = 0; j < Group2.pos.size(); j++) //for each object in the other group
		{
			if (Group1.pos[i] != Group2.pos[j])
			{
				//direct multiplication is more efficient then pow
				//			         this is inefficient, why compute that difference twice when we could compute it once
				const float distance_squared = ((Group1.pos[i].x - Group2.pos[j].x) * (Group1.pos[i].x - Group2.pos[j].x))
											 + ((Group1.pos[i].y - Group2.pos[j].y) * (Group1.pos[i].y - Group2.pos[j].y));
				// pre compute radius squared
				const float force = distance_squared < radius * radius ? 1.0F / std::sqrtf(distance_squared) : 0.0F;
				//re use pre computerd diffs here
				fx += ((Group1.pos[i].x - Group2.pos[j].x) * force);
				fy += ((Group1.pos[i].y - Group2.pos[j].y) * force);
			}
		}

		// Wall Repel
		if (wallRepel > 0.0F)
		{
			Group1.vel[i].x += Group1.pos[i].x < wallRepel ? (wallRepel - Group1.pos[i].x) * 0.1 : 0.0F;	// x
			Group1.vel[i].y += Group1.pos[i].y < wallRepel ? (wallRepel - Group1.pos[i].y) * 0.1 : 0.0F;	// x
			Group1.vel[i].x += Group1.pos[i].x > boundWidth - wallRepel  ? (boundWidth - wallRepel - Group1.pos[i].x) * 0.1  : 0.0F; // y 
			Group1.vel[i].y += Group1.pos[i].y > boundHeight - wallRepel ? (boundHeight - wallRepel - Group1.pos[i].y) * 0.1 : 0.0F; // y			
		}

		// Viscosity & gravity
		//perhaps store this computation of viscosity instead of what is currently stored
		//sture the computed velocities locally and reduce them together after each thread for a specific color finishes its job
		Group1.vel[i].x = (Group1.vel[i].x + (fx * g)) * (1.0 - viscosity);
		Group1.vel[i].y = (Group1.vel[i].y + (fy * g)) * (1.0 - viscosity) + worldGravity;

		//Update position do this outsize of the loop, after each thread does its thing, loop through every point and update this value
		//TODO use the opperator overload for this
		Group1.pos[i].x += Group1.vel[i].x;
		Group1.pos[i].y += Group1.vel[i].y;
	}

	// if "bounded" is checked then keep particles inside the window
	if (boundsToggle) {
		for (auto& p : Group1.pos)
		{
			p.x = std::min(std::max(p.x, 0.0F), static_cast<float>(boundWidth));
			p.y = std::min(std::max(p.y, 0.0F), static_cast<float>(boundHeight));
		}
	}
	
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
	if (numberSliderG > 0) { green  = CreatePoints(numberSliderG, ofColor::green); }
	if (numberSliderR > 0) { red    = CreatePoints(numberSliderR,   ofColor::red);   }
	if (numberSliderO > 0) { orange  = CreatePoints(numberSliderO, ofColor::orange); }
	if (numberSliderC > 0) { cyan = CreatePoints(numberSliderC,  ofColor::cyan); }

	vbo.setVertexData(green.pos.data(),  green.pos.size(),  GL_STREAM_DRAW);
	vbo.setVertexData(red.pos.data(),    red.pos.size(),    GL_STREAM_DRAW);
	vbo.setVertexData(orange.pos.data(),  orange.pos.size(),  GL_STREAM_DRAW);
	vbo.setVertexData(cyan.pos.data(), cyan.pos.size(), GL_STREAM_DRAW);
}


/**
 * @brief Generate initial simulation parameters
 */
void ofApp::random()
{
	// GREEN
	//numberSliderG = RandomFloat(0, 3000);
	powerSliderGG = RandomFloat(-100, 100) * forceVariance;
	powerSliderGR = RandomFloat(-100, 100) * forceVariance;
	powerSliderGO = RandomFloat(-100, 100) * forceVariance;
	powerSliderGC = RandomFloat(-100, 100) * forceVariance;

	vSliderGG = RandomFloat(10, 200) * radiusVariance;
	vSliderGR = RandomFloat(10, 200) * radiusVariance;
	vSliderGO = RandomFloat(10, 200) * radiusVariance;
	vSliderGC = RandomFloat(10, 200) * radiusVariance;

	// RED
	//numberSliderR = RandomFloat(0, 3000);
	powerSliderRR = RandomFloat(-100, 100) * forceVariance;
	powerSliderRG = RandomFloat(-100, 100) * forceVariance;
	powerSliderRO = RandomFloat(-100, 100) * forceVariance;
	powerSliderRC = RandomFloat(-100, 100) * forceVariance;

	vSliderRG = RandomFloat(10, 200) * radiusVariance;
	vSliderRR = RandomFloat(10, 200) * radiusVariance;
	vSliderRO = RandomFloat(10, 200) * radiusVariance;
	vSliderRC = RandomFloat(10, 200) * radiusVariance;

	// WHITE
	// numberSliderW = RandomFloat(0, 3000);
	powerSliderOO = RandomFloat(-100, 100) * forceVariance;
	powerSliderOR = RandomFloat(-100, 100) * forceVariance;
	powerSliderOG = RandomFloat(-100, 100) * forceVariance;
	powerSliderOC = RandomFloat(-100, 100) * forceVariance;

	vSliderOG = RandomFloat(10, 200) * radiusVariance;
	vSliderOR = RandomFloat(10, 200) * radiusVariance;
	vSliderOO = RandomFloat(10, 200) * radiusVariance;
	vSliderOC = RandomFloat(10, 200) * radiusVariance;

	// yellow
	//numberSliderY = RandomFloat(0, 3000);
	powerSliderCC = RandomFloat(-100, 100) * forceVariance;
	powerSliderCO = RandomFloat(-100, 100) * forceVariance;
	powerSliderCR = RandomFloat(-100, 100) * forceVariance;
	powerSliderCG = RandomFloat(-100, 100) * forceVariance;

	vSliderCG = RandomFloat(10, 200) * radiusVariance;
	vSliderCR = RandomFloat(10, 200) * radiusVariance;
	vSliderCO = RandomFloat(10, 200) * radiusVariance;
	vSliderCC = RandomFloat(10, 200) * radiusVariance;
}

/// this is a cheap and quick way to save and load parameters (openFramework have betters ways but requires some additional library setups) 
// Dialog gui tested on windows machine only. Not sure if it works on Mac or Linux too.
void ofApp::saveSettings()
{
	const std::vector<float> settings = {
		powerSliderGG, powerSliderGR, powerSliderGO, powerSliderGC,
		vSliderGG, vSliderGR, vSliderGO, vSliderGC,
		powerSliderRG, powerSliderRR, powerSliderRO, powerSliderRC,
		vSliderRG, vSliderRR, vSliderRO, vSliderRC,
		powerSliderOG, powerSliderOR, powerSliderOO, powerSliderOC,
		vSliderOG, vSliderOR, vSliderOO, vSliderOC,
		powerSliderCG, powerSliderCR, powerSliderCO, powerSliderCC,
		vSliderCG, vSliderCR, vSliderCO, vSliderCC,
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
		powerSliderGG = p[0];
		powerSliderGR = p[1];
		powerSliderGO = p[2];
		powerSliderGC = p[3];
		vSliderGG = p[4];
		vSliderGR = p[5];
		vSliderGO = p[6];
		vSliderGC = p[7];
		powerSliderRG = p[8];
		powerSliderRR = p[9];
		powerSliderRO = p[10];
		powerSliderRC = p[11];
		vSliderRG = p[12];
		vSliderRR = p[13];
		vSliderRO = p[14];
		vSliderRC = p[15];
		powerSliderOG = p[16];
		powerSliderOR = p[17];
		powerSliderOO = p[18];
		powerSliderOC = p[19];
		vSliderOG = p[20];
		vSliderOR = p[21];
		vSliderOO = p[22];
		vSliderOC = p[23];
		powerSliderCG = p[24];
		powerSliderCR = p[25];
		powerSliderCO = p[26];
		powerSliderCC = p[27];
		vSliderCG = p[28];
		vSliderCR = p[29];
		vSliderCO = p[30];
		vSliderCC = p[31];
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
	greenGroup.add(powerSliderGG.setup("green x green:", ppowerSliderGG, -100, 100));
	greenGroup.add(powerSliderGR.setup("green x red:", ppowerSliderGR, -100, 100));
	greenGroup.add(powerSliderGO.setup("green x orange:", ppowerSliderGO, -100, 100));
	greenGroup.add(powerSliderGC.setup("green x cyan:", ppowerSliderGC, -100, 100));

	greenGroup.add(vSliderGG.setup("radius g x g:", pvSliderGG, 10, 500));
	greenGroup.add(vSliderGR.setup("radius g x r:", pvSliderGR, 10, 500));
	greenGroup.add(vSliderGO.setup("radius g x o:", pvSliderGO, 10, 500));
	greenGroup.add(vSliderGC.setup("radius g x c:", pvSliderGC, 10, 500));

	greenGroup.minimize();
	gui.add(&greenGroup);

	// RED
	redGroup.setup("Red");
	redGroup.add(powerSliderRR.setup("red x red:", ppowerSliderRR, -100, 100));
	redGroup.add(powerSliderRG.setup("red x green:", ppowerSliderRG, -100, 100));
	redGroup.add(powerSliderRO.setup("red x orange:", ppowerSliderRO, -100, 100));
	redGroup.add(powerSliderRC.setup("red x cyan:", ppowerSliderRC, -100, 100));

	redGroup.add(vSliderRG.setup("radius r x g:", pvSliderRG, 10, 500));
	redGroup.add(vSliderRR.setup("radius r x r:", pvSliderRR, 10, 500));
	redGroup.add(vSliderRO.setup("radius r x o:", pvSliderRO, 10, 500));
	redGroup.add(vSliderRC.setup("radius r x c:", pvSliderRC, 10, 500));

	redGroup.minimize();
	gui.add(&redGroup);

	// WHITE
	ornageGroup.setup("Orange");
	ornageGroup.add(powerSliderOO.setup("Orange x Orange:", ppowerSliderOO, -100, 100));
	ornageGroup.add(powerSliderOR.setup("Orange x red:", ppowerSliderOR, -100, 100));
	ornageGroup.add(powerSliderOG.setup("Orange x green:", ppowerSliderOG, -100, 100));
	ornageGroup.add(powerSliderOC.setup("Orange x Cyan:", ppowerSliderOC, -100, 100));

	ornageGroup.add(vSliderOG.setup("radius w x g:", pvSliderOG, 10, 500));
	ornageGroup.add(vSliderOR.setup("radius w x r:", pvSliderOR, 10, 500));
	ornageGroup.add(vSliderOO.setup("radius w x o:", pvSliderOO, 10, 500));
	ornageGroup.add(vSliderOC.setup("radius w x c:", pvSliderOC, 10, 500));

	ornageGroup.minimize();
	gui.add(&ornageGroup);

	// yellow
	cyanGroup.setup("Cyan");
	cyanGroup.add(powerSliderCC.setup("Cyan x Cyan:", ppowerSliderCC, -100, 100));
	cyanGroup.add(powerSliderCO.setup("Cyan x Orange:", ppowerSliderCO, -100, 100));
	cyanGroup.add(powerSliderCR.setup("Cyan x red:", ppowerSliderCR, -100, 100));
	cyanGroup.add(powerSliderCG.setup("Cyan x green:", ppowerSliderCG, -100, 100));

	cyanGroup.add(vSliderCG.setup("radius y x g:", pvSliderCG, 10, 500));
	cyanGroup.add(vSliderCR.setup("radius y x r:", pvSliderCR, 10, 500));
	cyanGroup.add(vSliderCO.setup("radius y x o:", pvSliderCO, 10, 500));
	cyanGroup.add(vSliderCC.setup("radius y x c:", pvSliderCC, 10, 500));

	cyanGroup.minimize();
	gui.add(&cyanGroup);

	expGroup.setup("Experimental");
	expGroup.add(evoToggle.setup("Evolve parameters", false));
	expGroup.add(evoProbSlider.setup("evo chance%", evoChance, 0, 100));
	expGroup.add(evoAmountSlider.setup("evo amount%%", evoAmount, 0, 100));
	//expGroup.add(radiusToogle.setup("infinite radius", false));
	//expGroup.add(probabilitySlider.setup("interaction prob%", probability, 1, 100));
	expGroup.add(motionBlurToggle.setup("Motion Blur", false));
	expGroup.add(physicLabel.setup("physic (ms)", "0"));

	expGroup.minimize();
	gui.add(&expGroup);

	ofSetBackgroundAuto(false);
	//ofDisableAlphaBlending();

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

		interaction(red,   red,   powerSliderRR, vSliderRR, boundsToggle); 
		interaction(red,   green, powerSliderRR, vSliderRG, boundsToggle);
		interaction(red,   cyan,  powerSliderRR, vSliderRC, boundsToggle);
		interaction(red,   orange, powerSliderRR, vSliderRO, boundsToggle);
		interaction(green, red,   powerSliderGR, vSliderGR, boundsToggle);
		interaction(green, green, powerSliderGG, vSliderGG, boundsToggle);
		interaction(green, cyan,  powerSliderGC, vSliderGC, boundsToggle);
		interaction(green, orange, powerSliderGO, vSliderGO, boundsToggle);
		interaction(cyan,  red,   powerSliderCR, vSliderCR, boundsToggle);
		interaction(cyan,  green, powerSliderCG, vSliderCG, boundsToggle);
		interaction(cyan,  cyan,  powerSliderCC, vSliderCC, boundsToggle);
		interaction(cyan,  orange, powerSliderCO, vSliderCO, boundsToggle);
		interaction(orange, red,   powerSliderOR, vSliderOR, boundsToggle);
		interaction(orange, green, powerSliderOG, vSliderOG, boundsToggle);
		interaction(orange, cyan,  powerSliderOC, vSliderOC, boundsToggle);
		interaction(orange, orange, powerSliderOO, vSliderOO, boundsToggle);
		
	
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
			
			if (numberSliderO > 0) { Draw(orange); }
			if (numberSliderR > 0) { Draw(red); }
			if (numberSliderG > 0) { Draw(green); }
			if (numberSliderC > 0) { Draw(cyan); }
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
