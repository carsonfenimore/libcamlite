#include <stdio.h>
#include <chrono>
#include <sstream>
#include <iomanip>

#include "libcamlite.hpp"

using namespace libcamlite;

class VidTest {
public:
	VidTest(){}
	
	void setup(){
		using namespace std::placeholders;  // for _1, _2, _3...
						    //
		H264Params h264Config;

		h264Config.stream.width = 1280;
		h264Config.stream.height = 720;
		h264Config.bitrate = "2mbps";
		h264Config.intraPeriod = 5;
		h264Config.stream.framerate = 30;
		h264Config.profile = "high";
		libcam.setupH264Stream(h264Config, std::bind(&VidTest::h264Callback, this, _1, _2, _3, _4));

		LowResParams lowresConfig;
		lowresConfig.stream.width = 300;
		lowresConfig.stream.height= 300;
		libcam.setupLowresStream(lowresConfig, std::bind(&VidTest::lowresCallback, this, _1, _2));
	}

	void run(){
		libcam.start();
	}


private:
	LibCamLite libcam;

	unsigned int bytesTotal = 0;
	unsigned int framesTotal = 0;
	const unsigned int FPS_REPORT_SECS = 2;
	std::chrono::time_point<std::chrono::steady_clock> last =  std::chrono::steady_clock::now();

	unsigned int detectionTotal = 0;
	unsigned int detectionGlobal = 0;
	const unsigned int DETECT_REPORT_SECS = 2;
	std::chrono::time_point<std::chrono::steady_clock> lastDetect =  std::chrono::steady_clock::now();

	void h264Callback(uint8_t* mem, size_t size, int64_t timestamp_us, bool keyframe){
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

	void lowresCallback(uint8_t* mem, size_t size) {
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

int main(int argc, char** argv){
	VidTest test;
	test.setup();
	test.run();

	return 0;
}
