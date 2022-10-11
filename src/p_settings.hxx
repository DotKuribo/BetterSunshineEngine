#include <Dolphin/types.h>
#include <JSystem/JDrama/JDRDirector.hxx>
#include <JSystem/JKernel/JKRHeap.hxx>

class SettingsDirector : public JDrama::TDirector {
    enum class State { INIT, GROUP, SETTING, EXIT };

public:
    SettingsDirector() : TDirector("SettingsDirector"), mScreen() {}
    ~SettingsDirector() override {}

    s32 direct() override;

private:
    void initialize();
    void initializeLayout();

    void processInput();
    void processAnimations();

    void draw();
    s32 exit();

private:
    State mState;
    JKRSolidHeap *mHeap;
    J2DScreen mScreen;
    s32 mGroupID;
    s32 mSettingID;
    JGadget::TList<J2DPane *> mGroupPanes;
    JGadget::TList<SettingsGroup *> mGroups;
};