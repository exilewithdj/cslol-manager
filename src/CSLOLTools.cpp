#include <QDebug>
#include <chrono>

#include "CSLOLTools.h"
#include "CSLOLToolsImpl.h"

CSLOLTools::CSLOLTools(QObject *parent) : QObject(parent) {
    //csloltool 这个c++类 也是 qt组件的构造函数
    qRegisterMetaType<CSLOLState>("CSLOLState");
    thread_ = new QThread();
    worker_ = new CSLOLToolsImpl();
    // movetothread 移动到 子线程中
    worker_->moveToThread(thread_);
    // [第零阶段：用户操作触发]
    // [主线程] QML界面
    //     ↓ 用户点击保存按钮
    //     ↓ 触发ToolButton的onClicked事件
    //     ↓ 调用cslolToolBar.saveProfileAndRun(true)
    //     ↓ 发送saveProfile信号
    //------第一阶段(主线程 -> 工作线程)------------
    // [主线程] CSLOLTools (this) 
    // ↓ 发送saveProfile信号
    // [Qt信号槽系统]    自动处理线程间通信  信号:CSLOLTools::saveProfile  槽函数: CSLOLToolsImpl::saveProfile  
    // ↓ 将保存操作的任务转交给工作进程
    // [工作线程] CSLOLToolsImpl (worker_)
    // ↓ 执行saveProfile槽函数  保存操作
    //-------第二阶段(工作线程-> 主线程)-----------
    // [工作线程] CSLOLToolsImpl::saveProfile完成
    //     ↓ 发送profileSaved信号
    // [Qt信号槽系统] 自动处理线程切换   信号： CSLOLToolsImpl::profileSaved 槽函数： CSLOLTools::profileSaved
    //     ↓ 将信号转换为事件放入主线程队列
    // [主线程] CSLOLTools::profileSaved
    //     ↓ 接收信号并处理
    //--------第三段流程（主线程 -> QML界面）---------
    // [主线程] CSLOLTools::profileSaved
    // ↓ 发送profileSaved信号
    // [Qt信号槽系统] 自动处理QML信号  信号：CSLOLTools::profileSaved 槽函数：onProfileSaved
    //     ↓ 触发QML信号处理器
    // [主线程] QML onProfileSaved
    //     ↓ 执行回调函数
    //     ↓ console.log("Profile saved:", name)
    //     ↓ window.showSuccess("配置 '" + name + "' 保存成功！")
    connect(worker_, &CSLOLToolsImpl::stateChanged, this, &CSLOLTools::setState);
    connect(worker_, &CSLOLToolsImpl::statusChanged, this, &CSLOLTools::setStatus);
    connect(worker_, &CSLOLToolsImpl::leaguePathChanged, this, &CSLOLTools::setLeaguePath);
    connect(worker_, &CSLOLToolsImpl::reportError, this, &CSLOLTools::reportError);

    connect(worker_, &CSLOLToolsImpl::blacklistChanged, this, &CSLOLTools::blacklistChanged);
    connect(worker_, &CSLOLToolsImpl::ignorebadChanged, this, &CSLOLTools::ignorebadChanged);
    connect(worker_, &CSLOLToolsImpl::initialized, this, &CSLOLTools::initialized);
    connect(worker_, &CSLOLToolsImpl::modDeleted, this, &CSLOLTools::modDeleted);
    connect(worker_, &CSLOLToolsImpl::installedMod, this, &CSLOLTools::installedMod);
    connect(
        worker_,                    // 发送者：CSLOLToolsImpl对象
        &CSLOLToolsImpl::profileSaved,  // 信号：CSLOLToolsImpl类中的profileSaved信号
        this,                       // 接收者：当前对象（CSLOLTools）
        &CSLOLTools::profileSaved   // 槽函数：CSLOLTools类中的profileSaved槽函数
    );
    connect(worker_, &CSLOLToolsImpl::profileLoaded, this, &CSLOLTools::profileLoaded);
    connect(worker_, &CSLOLToolsImpl::profileDeleted, this, &CSLOLTools::profileDeleted);
    connect(worker_, &CSLOLToolsImpl::modCreated, this, &CSLOLTools::modCreated);
    connect(worker_, &CSLOLToolsImpl::modEditStarted, this, &CSLOLTools::modEditStarted);
    connect(worker_, &CSLOLToolsImpl::modInfoChanged, this, &CSLOLTools::modInfoChanged);
    connect(worker_, &CSLOLToolsImpl::modWadsAdded, this, &CSLOLTools::modWadsAdded);
    connect(worker_, &CSLOLToolsImpl::modWadsRemoved, this, &CSLOLTools::modWadsRemoved);
    connect(worker_, &CSLOLToolsImpl::updatedMods, this, &CSLOLTools::updatedMods);

    connect(this, &CSLOLTools::changeLeaguePath, worker_, &CSLOLToolsImpl::changeLeaguePath);
    connect(this, &CSLOLTools::changeBlacklist, worker_, &CSLOLToolsImpl::changeBlacklist);
    connect(this, &CSLOLTools::changeIgnorebad, worker_, &CSLOLToolsImpl::changeIgnorebad);
    connect(this, &CSLOLTools::init, worker_, &CSLOLToolsImpl::init);
    connect(this, &CSLOLTools::deleteMod, worker_, &CSLOLToolsImpl::deleteMod);
    connect(this, &CSLOLTools::exportMod, worker_, &CSLOLToolsImpl::exportMod);
    connect(this, &CSLOLTools::installFantomeZip, worker_, &CSLOLToolsImpl::installFantomeZip);
    connect(this, &CSLOLTools::saveProfile, worker_, &CSLOLToolsImpl::saveProfile);
    // this：当前CSLOLTools对象，在主线程中
    // worker_：CSLOLToolsImpl对象，在thread_工作线程中
    // 信号：CSLOLTools::saveProfile
    // 槽：CSLOLToolsImpl::saveProfile
    connect(this, &CSLOLTools::loadProfile, worker_, &CSLOLToolsImpl::loadProfile);
    connect(this, &CSLOLTools::deleteProfile, worker_, &CSLOLToolsImpl::deleteProfile);
    connect(this, &CSLOLTools::stopProfile, worker_, &CSLOLToolsImpl::stopProfile);
    connect(this, &CSLOLTools::makeMod, worker_, &CSLOLToolsImpl::makeMod);
    connect(this, &CSLOLTools::startEditMod, worker_, &CSLOLToolsImpl::startEditMod);
    connect(this, &CSLOLTools::changeModInfo, worker_, &CSLOLToolsImpl::changeModInfo);
    connect(this, &CSLOLTools::addModWad, worker_, &CSLOLToolsImpl::addModWad);
    connect(this, &CSLOLTools::removeModWads, worker_, &CSLOLToolsImpl::removeModWads);
    connect(this, &CSLOLTools::refreshMods, worker_, &CSLOLToolsImpl::refreshMods);
    connect(this, &CSLOLTools::runDiag, worker_, &CSLOLToolsImpl::runDiag);

    connect(this, &CSLOLTools::destroyed, worker_, &CSLOLToolsImpl::deleteLater);
    connect(worker_, &CSLOLTools::destroyed, thread_, &QThread::deleteLater);

    thread_->start();
}

CSLOLTools::~CSLOLTools() {}

CSLOLToolsImpl::CSLOLState CSLOLTools::getState() { return state_; }

QString CSLOLTools::getStatus() { return status_; }

void CSLOLTools::setState(CSLOLState value) {
    if (state_ != value) {
        state_ = value;
        emit stateChanged(value);
    }
}

void CSLOLTools::setStatus(QString status) {
    if (status_ != status) {
        status_ = status;
        emit statusChanged(status);
    }
}

QString CSLOLTools::getLeaguePath() { return leaguePath_; }

void CSLOLTools::setLeaguePath(QString value) {
    if (leaguePath_ != value) {
        leaguePath_ = value;
        emit leaguePathChanged(value);
    }
}
