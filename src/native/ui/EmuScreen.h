#pragma once
#include <string>
#include <vector>
#include <list>
#include "input/keycodes.h"
#include "ui/screen.h"
#include "ui/ui_screen.h"
#include "Common/KeyMap.h"

#define UBUNTU24        0
#define I_SOLIDWHITE    0
#define I_CROSS         1
#define I_CIRCLE        2
#define I_SQUARE        3
#define I_TRIANGLE      4
#define I_SELECT        5
#define I_START         6
#define I_ARROW         7
#define I_DIR           8
#define I_ROUND         9
#define I_RECT          10
#define I_STICK         11
#define I_STICK_BG      12
#define I_SHOULDER      13
#define I_DIR_LINE      14
#define I_ROUND_LINE    15
#define I_RECT_LINE     16
#define I_SHOULDER_LINE 17
#define I_STICK_LINE    18
#define I_STICK_BG_LINE 19
#define I_CHECKEDBOX    20
#define I_BG            21
#define I_BG_GOLD       22
#define I_L             23
#define I_R             24
#define I_DROP_SHADOW   25
#define I_LINES         26
#define I_GRID          27
#define I_LOGO          28
#define I_ICON          29
#define I_ICONGOLD      30
#define I_FOLDER        31
#define I_UP_DIRECTORY  32
#define I_GEAR          33


struct AxisInput;
class AsyncImageFileView;
class EmuScreen : public UIScreen {
  public:
    EmuScreen(const std::string &filename);
    ~EmuScreen();

    virtual void update(InputState &input) override;
    virtual void render() override;
    virtual void deviceLost() override;
    virtual void dialogFinished(const Screen *dialog, DialogResult result) override;
    virtual void sendMessage(const char *msg, const char *value) override;
    virtual bool touch(const TouchInput &touch) override;
    virtual bool key(const KeyInput &key) override;
    virtual bool axis(const AxisInput &axis) override;

  protected:
    virtual void CreateViews(){};

  private:
    void bootGame(const std::string &filename);
    void bootComplete();
    void pspKey(int pspKeyCode, int flags);
    void autoLoad();
    void checkPowerDown();

  private:
    bool            bootPending_;
    std::string     gamePath_;
    bool            invalid_;
    bool            quit_;
    std::string     errorMessage_;
    bool            pauseTrigger_;
    bool            virtKeys[VIRTKEY_COUNT];
    std::vector<u8> freezeState_;
    std::string     tag_;
    int             axisState_[JOYSTICK_AXIS_MAX];

    double              saveStatePreviewShownTime_;
    AsyncImageFileView *saveStatePreview_;
};
