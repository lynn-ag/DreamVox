#include <juce_audio_utils/juce_audio_utils.h>
#include <iostream>
#ifdef DREAM_VOX
#include "../DreamVox-v0.1/DreamVox/Source/PluginProcessor.h"
using Processor=DreamVoxAudioProcessor;
constexpr int expectedKnobs=8;
#else
#include "../DreamMaster/Source/PluginProcessor.h"
using Processor=DreamMasterProcessor;
constexpr int expectedKnobs=9;
#endif
static void require(bool success,const char* message) {if(!success)throw std::runtime_error(message);}
int main(int argc,char** argv) {
    if(argc!=2)return 1;
    try {
        juce::ScopedJuceInitialiser_GUI init;
        Processor processor;processor.prepareToPlay(48000,512);
        juce::MemoryBlock initial;processor.getStateInformation(initial);
        std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
        int count=0;
        for(auto* child:editor->getChildren())if(auto* slider=dynamic_cast<juce::Slider*>(child)) {
            ++count;
            auto* parameter=processor.parameters.getParameter(slider->getProperties()["parameterId"].toString());
            require(parameter!=nullptr,"Missing slider parameter");
            const float desired=parameter->convertFrom0to1(.7f);
            slider->setValue(desired,juce::sendNotificationSync);
            require(std::abs(parameter->convertFrom0to1(parameter->getValue())-desired)<.03f,"Knob did not update audio parameter");
            parameter->setValueNotifyingHost(.3f);
            require(std::abs(slider->getValue()-parameter->convertFrom0to1(parameter->getValue()))<.03,"Automation did not update knob");
        }
        require(count==expectedKnobs,"Unexpected knob count");
        processor.setStateInformation(initial.getData(),int(initial.getSize()));
        editor.reset();editor.reset(processor.createEditor());
        for(auto* child:editor->getChildren())if(auto* slider=dynamic_cast<juce::Slider*>(child)) {
            auto* p=processor.parameters.getParameter(slider->getProperties()["parameterId"].toString());
            require(std::abs(slider->getValue()-p->convertFrom0to1(p->getDefaultValue()))<.03,"State did not restore control");
        }
        juce::AudioBuffer<float> audio(2,512);juce::MidiBuffer midi;
        for(int block=0;block<100;++block){for(int c=0;c<2;++c)for(int n=0;n<512;++n)audio.setSample(c,n,.1f*std::sin(float(block*512+n)*.08f));processor.processBlock(audio,midi);}
        const auto path=juce::File::getCurrentWorkingDirectory().getChildFile(juce::String::fromUTF8(argv[1]));
        for(int width:{1200,960,1500}) {
            editor->setSize(width,width*780/1200);
            for(auto* child:editor->getChildren())if(dynamic_cast<juce::Slider*>(child)||dynamic_cast<juce::ComboBox*>(child)||dynamic_cast<juce::ToggleButton*>(child)) {
                require(editor->getLocalBounds().contains(child->getBounds()),"Control outside editor at resized dimensions");
            }
            const auto image=editor->createComponentSnapshot(editor->getLocalBounds());
            require(image.isValid(),"Invalid editor image");
            const auto out=width==1200?path:path.getSiblingFile(path.getFileNameWithoutExtension()+"-"+juce::String(width)+".png");
            auto stream=out.createOutputStream();require(stream!=nullptr,"Cannot create snapshot");
            require(juce::PNGImageFormat().writeImageToStream(image,*stream),"Cannot write snapshot");
        }
        std::cout<<"PASS: "<<count<<" attached knobs, bidirectional automation, state recall, editor reopen, audio processing, bounds and images at 80/100/125 percent.\n";
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 2;}
}
