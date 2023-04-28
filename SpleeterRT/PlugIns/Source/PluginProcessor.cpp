#include <JuceHeader.h>
//#define GUI_DEBUG
#ifdef GUI_DEBUG
int isStandalone;
#endif
extern "C"
{
#include "Spleeter4Stems.h"
}
enum { PARAM_ORDER, PARAM__MAX };
const char paramName[PARAM__MAX][2][12] =
{
{"Channel", "order" },
};
const float paramValueRange[PARAM__MAX][5] =
{
	{ 0.0f, 3.0f, 0.0f, 1.0f, 1.0f }
};
#ifdef _WIN32
    #include <userenv.h>  // GetUserProfileDirectory() (link with userenv)
    const int BUFLEN = 512;
    BOOL getCurrentUserDir(char* buf, DWORD buflen)
    {
        HANDLE hToken;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hToken))
            return FALSE;
        if (!GetUserProfileDirectory(hToken, buf, &buflen))
            return FALSE;
        CloseHandle(hToken);
        return TRUE;
    }
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>
    bool getCurrentUserDirUnix(char* buf, int buflen)
    {
        if (getenv("HOME") == NULL)
            strcpy(buf, getpwuid(getuid())->pw_dir);
        else
            strcpy(buf, getenv("HOME"));
        return true;
    }
#endif
class MS5_1AI : public AudioProcessor, public AudioProcessorValueTreeState::Listener
{
public:
	//==============================================================================
	MS5_1AI() : AudioProcessor(), treeState(*this, nullptr, "Parameters", createParameterLayout())
	{
#ifdef GUI_DEBUG
		isStandalone = JUCEApplicationBase::isStandaloneApp();
#endif
		int i;
		for (i = 0; i < PARAM__MAX; i++)
			treeState.addParameterListener(paramName[i][0], this);
		fs = 44100;
		order = 0;
		char dir[512];
#ifdef _WIN32
		getCurrentUserDir(dir, 512);
#else
        getCurrentUserDirUnix(dir, 512);
#endif
		for (i = 0; i < 4; i++)
			coeffProvPtr[i] = malloc(getCoeffSize());
		// Load coeff
#ifdef _WIN32
		char file1[17] = "\\drum4stems.dat";
		char file2[17] = "\\bass4stems.dat";
		char file3[26] = "\\accompaniment4stems.dat";
		char file4[17] = "\\vocal4stems.dat";
#else
        char file1[17] = "/drum4stems.dat";
        char file2[17] = "/bass4stems.dat";
        char file3[26] = "/accompaniment4stems.dat";
        char file4[17] = "/vocal4stems.dat";
#endif
		size_t len1 = strlen(dir);
		size_t len2 = strlen(file1);
		char *concat = (char*)malloc(len1 + len2 + 1);
		memcpy(concat, dir, len1);
		memcpy(concat + len1, file1, len2 + 1);
		FILE *fp = fopen(concat, "rb");
		fread(coeffProvPtr[0], 1, 39290900, fp);
		fclose(fp);
		free(concat);
		len2 = strlen(file2);
		concat = (char*)malloc(len1 + len2 + 1);
		memcpy(concat, dir, len1);
		memcpy(concat + len1, file2, len2 + 1);
		fp = fopen(concat, "rb");
		fread(coeffProvPtr[1], 1, 39290900, fp);
		fclose(fp);
		free(concat);
		len2 = strlen(file3);
		concat = (char*)malloc(len1 + len2 + 1);
		memcpy(concat, dir, len1);
		memcpy(concat + len1, file3, len2 + 1);
		fp = fopen(concat, "rb");
		fread(coeffProvPtr[2], 1, 39290900, fp);
		fclose(fp);
		free(concat);
		len2 = strlen(file4);
		concat = (char*)malloc(len1 + len2 + 1);
		memcpy(concat, dir, len1);
		memcpy(concat + len1, file4, len2 + 1);
		fp = fopen(concat, "rb");
		fread(coeffProvPtr[3], 1, 39290900, fp);
		fclose(fp);
		free(concat);
		setLatencySamples(16384);
		msr = 0;
	}
	~MS5_1AI()
	{
        if(msr)
            Spleeter4StemsFree(msr);
		if (coeffProvPtr[0])
		{
			for (int i = 0; i < 4; i++)
				free(coeffProvPtr[i]);
			coeffProvPtr[0] = 0;
		}
	}
	AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
	{
		std::vector<std::unique_ptr<RangedAudioParameter>> parameters;
		for (int i = 0; i < PARAM__MAX; i++)
        parameters.push_back(std::make_unique<AudioParameterFloat>(ParameterID{paramName[i][0], 1}, paramName[i][0], NormalisableRange<float>(paramValueRange[i][0], paramValueRange[i][1], paramValueRange[i][4]), paramValueRange[i][2]));
		return { parameters.begin(), parameters.end() };
	}
	void parameterChanged(const String& parameter, float newValue) override
	{
		if (parameter == paramName[0][0])
		{
			order = (int)newValue;
		}
	}
	//==============================================================================
	void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override
	{
		fs = (int)sampleRate;
		if (msr)
		{
			Spleeter4StemsFree(msr);
			free(msr);
		}
		msr = (Spleeter4Stems*)malloc(sizeof(Spleeter4Stems));
		Spleeter4StemsInit(msr, 1536, 256, coeffProvPtr);
	}
	void releaseResources() override
	{
	}

	void processBlock(AudioBuffer<float>& buffer, MidiBuffer&) override
	{
#ifdef GUI_DEBUG
		if (isStandalone)
			return;
#endif
		// number of samples per buffer
		const int n = buffer.getNumSamples();
		// input channels
		const float *inputs[2] = { buffer.getReadPointer(0), buffer.getReadPointer(1) };
		float *tmpPtr1, *tmpPtr2;

        if (buffer.getNumChannels() == 8) {
            float *outputs[8] = {buffer.getWritePointer(0), buffer.getWritePointer(1),
                                 buffer.getWritePointer(2), buffer.getWritePointer(3), buffer.getWritePointer(4),
                                 buffer.getWritePointer(5), buffer.getWritePointer(6), buffer.getWritePointer(7)};
            if (order == 1) {
                tmpPtr1 = outputs[0];
                tmpPtr2 = outputs[1];
                outputs[0] = outputs[2];
                outputs[1] = outputs[3];
                outputs[2] = tmpPtr1;
                outputs[3] = tmpPtr2;
            } else if (order == 2) {
                tmpPtr1 = outputs[0];
                tmpPtr2 = outputs[1];
                outputs[0] = outputs[4];
                outputs[1] = outputs[5];
                outputs[4] = tmpPtr1;
                outputs[5] = tmpPtr2;
            } else if (order == 3) {
                tmpPtr1 = outputs[0];
                tmpPtr2 = outputs[1];
                outputs[0] = outputs[6];
                outputs[1] = outputs[7];
                outputs[6] = tmpPtr1;
                outputs[7] = tmpPtr2;
            }
            // output channels
            int i, offset = 0;
            while (offset < n) {
                float *ptr[8] = {outputs[0] + offset, outputs[1] + offset,
                                 outputs[2] + offset, outputs[3] + offset, outputs[4] + offset,
                                 outputs[5] + offset, outputs[6] + offset, outputs[7] + offset};
                const int processing = min(n - offset, OVPSIZE);
                Spleeter4StemsProcessSamples(msr, inputs[0] + offset, inputs[1] + offset, processing, ptr);
                offset += processing;
            }
        }
	}

	//==============================================================================
	AudioProcessorEditor* createEditor() override { return new GenericAudioProcessorEditor(*this); }
	bool hasEditor() const override { return true; }
	const String getName() const override { return "SpleeterRTPlug"; }
	bool acceptsMidi() const override { return false; }
	bool producesMidi() const override { return false; }
	double getTailLengthSeconds() const override { return 0.0; }
	int getNumPrograms() override { return 1; }
	int getCurrentProgram() override { return 0; }
	void setCurrentProgram(int) override {}
	const String getProgramName(int) override { return {}; }
	void changeProgramName(int, const String&) override {}
	bool isVST2() const noexcept { return (wrapperType == wrapperType_VST); }

	//==============================================================================
	inline double map(double x, double in_min, double in_max, double out_min, double out_max)
	{
		return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	}
	void getStateInformation(MemoryBlock& destData) override
		{
			MemoryOutputStream stream(destData, true);
			for (int i = 0; i < PARAM__MAX; i++)
				stream.writeFloat(treeState.getParameter(paramName[i][0])->getValue());
		}
		void setStateInformation(const void* data, int sizeInBytes) override
		{
			MemoryInputStream stream(data, static_cast<size_t> (sizeInBytes), false);
			for (int i = 0; i < PARAM__MAX; i++)
				treeState.getParameter(paramName[i][0])->setValueNotifyingHost(stream.readFloat());
		}
private:
	int isStandalone;
	//==============================================================================
	void *coeffProvPtr[4];
	Spleeter4Stems *msr;
	int fs;
	int order;
	AudioProcessorValueTreeState treeState;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MS5_1AI)
};
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new MS5_1AI();
}
