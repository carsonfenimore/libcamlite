#include <stdio.h>
#include <chrono>
#include <sstream>
#include <iomanip>

#include "libcamlite.hpp"

using namespace std::placeholders;

class VidTest {
private:
	std::unique_ptr<LibCamLite> app;

	unsigned int bytesTotal = 0;
	unsigned int framesTotal = 0;
	const unsigned int FPS_REPORT_SECS = 2;
	std::chrono::time_point<std::chrono::steady_clock> last =  std::chrono::steady_clock::now();

	CamParam h264Config;
	CamParam lowresConfig;

	unsigned int detectionTotal = 0;
	unsigned int detectionGlobal = 0;
	const unsigned int DETECT_REPORT_SECS = 2;
	std::chrono::time_point<std::chrono::steady_clock> lastDetect =  std::chrono::steady_clock::now();

public:
	VidTest(){
		h264Config.width = 1280;
		h264Config.height = 720;
		h264Config.h264Bitrate = "2mbps";
		h264Config.h264IntraPeriod = 5;
		h264Config.framerate = 30;
		h264Config.h264Profile = "high";
		lowresConfig.width = 300;
		lowresConfig.height= 300;

		app = std::make_unique<LibCamLite>(h264Config, std::bind(&VidTest::h264Callback, this, _1, _2, _3, _4), 
				lowresConfig, std::bind(&VidTest::lowresCallback, this, _1)); 
	}

	void run(){
		app->start();
	}

private:
	void h264Callback(void* mem, size_t size, int64_t timestamp_us, bool keyframe){
		auto now =  std::chrono::steady_clock::now();
		auto delta = std::chrono::duration<double, std::milli>(now - last);
		float deltaSecs = delta.count()/1000.0;
		framesTotal++;
		bytesTotal += size;
		if (deltaSecs > FPS_REPORT_SECS){
			printf("VidTest: h264 received %.2f fps %d bytes/sec\n", framesTotal / deltaSecs, (int)(bytesTotal / deltaSecs));
			last = now;
			bytesTotal = 0;
			framesTotal = 0;
		}
	}

	std::string to_zero_lead(const int value, const unsigned precision)
	{
	     std::ostringstream oss;
	     oss << std::setw(precision) << std::setfill('0') << value;
	     return oss.str();
	}

	void lowresCallback(const std::vector<uint8_t>& mem){
		const unsigned int numChans = 3;
		//detect->detect(mem, lowresConfig.width, lowresConfig.height, numChans);
		auto now =  std::chrono::steady_clock::now();
		auto delta = std::chrono::duration<double, std::milli>(now - lastDetect);
		float deltaSecs = delta.count()/1000.0;
		detectionTotal++;
		detectionGlobal++;
		if (deltaSecs > DETECT_REPORT_SECS){
			printf("Vidtest: lowres received %.2f fps\n", detectionTotal / deltaSecs);
			lastDetect = now;
			detectionTotal= 0;
		}
	}
};

int main(int, char**){
	VidTest test;
	test.run();
	return 0;
}
