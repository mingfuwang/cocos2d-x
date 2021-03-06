#include "AppDelegate.h"
#include "PlayerStatus.h"
#include "cocos2d.h"
#include "SimpleAudioEngine.h"
#include "ScriptingCore.h"
#include "generated/cocos2dx.hpp"
#include "cocos2d_specifics.hpp"
#include "js_bindings_chipmunk_registration.h"
#include "js_bindings_ccbreader.h"
#include "MainSceneHelper.h"

#ifdef JSLOG
#undef JSLOG
#endif
#define JSLOG  CocosBuilder_log

char *_js_log_buf_ccbuilder = NULL;

USING_NS_CC;
using namespace CocosDenshion;

AppDelegate::AppDelegate()
{
}

AppDelegate::~AppDelegate()
{
    CCScriptEngineManager::sharedManager()->purgeSharedManager();
}

void handle_ccb_run() {
    CCFileUtils::sharedFileUtils()->purgeCachedEntries();
    ScriptingCore::getInstance()->runScript("main.js");
}

void handle_connected() {
  CCBHelper::setStatusMessage("Connected!");
}

void handle_disconnected() {
  CCBHelper::setStatusMessage("Disconnected");
}

void handle_ccb_stop() {
  CCLOG("STOP UNIMPLEMENTED");
}

extern "C" {

bool runMainScene() {
    PlayerStatus::loadMainScene("StatusLayer.ccbi");
    return true;
}

bool AppDelegate::applicationDidFinishLaunching()
{

    // initialize director
    CCDirector *pDirector = CCDirector::sharedDirector();
    pDirector->setOpenGLView(CCEGLView::sharedOpenGLView());
    pDirector->setProjection(kCCDirectorProjection2D);


    CCSize screenSize = CCEGLView::sharedOpenGLView()->getFrameSize();

    CCSize designSize = CCSizeMake(320, 480);
    CCSize resourceSize = CCSizeMake(320, 480);

    CCFileUtils* pFileUtils = CCFileUtils::sharedFileUtils();
    std::vector<std::string> searchResOrder;
    string res = "xlarge";
    // if (screenSize.height > 1024)
    // {
    //     resourceSize = CCSizeMake(1280, 1920);
    //     searchResOrder.push_back("resources-xlarge");
    //     res = "xlarge";
    // }
    // else
     if (screenSize.height > 960)
      {
    	resourceSize = CCSizeMake(640, 960);
        searchResOrder.push_back("resources-large");
    	res = "large";
      }
    else if (screenSize.height > 480)
      {
    	resourceSize = CCSizeMake(480, 720);
        searchResOrder.push_back("resources-medium");
    	res = "medium";
      }
    else
      {
    	resourceSize = CCSizeMake(320, 568);
        searchResOrder.push_back("resources-small");
    	res = "small";
      }

    pFileUtils->setSearchResolutionsOrder(searchResOrder);
    pDirector->setContentScaleFactor(resourceSize.height/designSize.height);

    CCEGLView::sharedOpenGLView()->setDesignResolutionSize(designSize.width, designSize.height, kResolutionNoBorder);

    std::vector<std::string> searchPaths = pFileUtils->getSearchPaths();
    searchPaths.insert(searchPaths.begin(), pFileUtils->getWritablePath());
    pFileUtils->setSearchPaths(searchPaths);

    PlayerStatus::setDeviceResolution(res);
    // turn on display FPS
    pDirector->setDisplayStats(true);
    
    // set FPS. the default value is 1.0/60 if you don't call this
    pDirector->setAnimationInterval(1.0 / 60);

    ScriptingCore* sc = ScriptingCore::getInstance();
    sc->addRegisterCallback(register_all_cocos2dx);
    sc->addRegisterCallback(register_cocos2dx_js_extensions);
    sc->addRegisterCallback(register_CCBuilderReader);
    sc->addRegisterCallback(jsb_register_chipmunk);
    
    sc->start();
    CCScriptEngineProtocol *pEngine = ScriptingCore::getInstance();
    CCScriptEngineManager::sharedManager()->setScriptEngine(pEngine);
    runMainScene();
}



void handle_signal(int signal) {
    static int internal_state = 0;
    ScriptingCore* sc = ScriptingCore::getInstance();
    // should start everything back
    CCDirector* director = CCDirector::sharedDirector();
    if (director->getRunningScene()) {
        director->popToRootScene();
    } else {
        CCPoolManager::sharedPoolManager()->finalize();
        if (internal_state == 0) {
            //sc->dumpRoot(NULL, 0, NULL);
            sc->start();
            internal_state = 1;
        } else {
            sc->runScript("hello.js");
            internal_state = 0;
        }
    }
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground()
{
    CCDirector::sharedDirector()->stopAnimation();
    SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();
    SimpleAudioEngine::sharedEngine()->pauseAllEffects();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground()
{
    CCDirector::sharedDirector()->startAnimation();
    SimpleAudioEngine::sharedEngine()->resumeBackgroundMusic();
    SimpleAudioEngine::sharedEngine()->resumeAllEffects();
}

}
