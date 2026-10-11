#include "../Source/PluginProcessor.h"
#include <iostream>
int main(int argc,char** argv)
{
    if(argc!=2) { std::cerr<<"Usage: DreamMasterPreview output.png\n";return 1; }
    juce::ScopedJuceInitialiser_GUI init;
    DreamMasterProcessor processor;
    processor.prepareToPlay(48000,512);
    processor.applyPreset(1);
    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    const auto image=editor->createComponentSnapshot(editor->getLocalBounds());
    const auto path=juce::File::getCurrentWorkingDirectory().getChildFile(juce::String::fromUTF8(argv[1]));
    auto stream=path.createOutputStream();
    if(!image.isValid()||!stream||!juce::PNGImageFormat().writeImageToStream(image,*stream))return 2;
    std::cout<<"Editor snapshot saved: "<<image.getWidth()<<" x "<<image.getHeight()<<"\n";
    return 0;
}
