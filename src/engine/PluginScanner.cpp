#include "PluginScanner.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <QDebug>
#include <vector>

namespace OpenDaw {

PluginScanWorker::PluginScanWorker(te::Engine& engine)
    : engine_(engine)
{
}

void PluginScanWorker::doScan()
{
    struct FormatInfo {
        juce::AudioPluginFormat* format;
        const char*              label;
    };

    juce::VST3PluginFormat vst3;
#if JUCE_PLUGINHOST_VST
    juce::VSTPluginFormat  vst2;
#endif

    std::vector<FormatInfo> formats = {
        { &vst3, "VST3" }
#if JUCE_PLUGINHOST_VST
        , { &vst2, "VST2" }
#endif
    };

#if JUCE_MAC
    // JUCE's defaults cover the standard locations, but keeping the paths
    // explicit makes the macOS scan predictable and covers per-user plugins.
    const auto userHome = juce::File::getSpecialLocation(
        juce::File::userHomeDirectory);
    const juce::File userVst3 = userHome.getChildFile(
        "Library/Audio/Plug-Ins/VST3");
    const juce::File systemVst3(
        "/Library/Audio/Plug-Ins/VST3");

    auto addPathIfPresent = [](juce::FileSearchPath& paths,
                               const juce::File& path) {
        if (path.isDirectory())
            paths.add(path.getFullPathName());
    };
#endif

#if JUCE_MAC && JUCE_PLUGINHOST_AU
    juce::AudioUnitPluginFormat audioUnit;
    formats.push_back({ &audioUnit, "Audio Unit" });
#endif

    juce::StringArray allFiles;
    juce::Array<juce::AudioPluginFormat*> fileFormats;

    for (auto& fmt : formats) {
        auto paths = fmt.format->getDefaultLocationsToSearch();

#if JUCE_MAC
        if (fmt.format == &vst3) {
            addPathIfPresent(paths, userVst3);
            addPathIfPresent(paths, systemVst3);
            qInfo() << "[PluginScanner] macOS VST3 paths:"
                    << QString::fromStdString(paths.toString().toStdString());
        }
#endif

        auto files = fmt.format->searchPathsForPlugins(paths, true, true);
        qInfo() << "[PluginScanner] scanning" << fmt.label << ":"
                << files.size() << "candidate(s)";
        for (auto& f : files) {
            allFiles.add(f);
            fileFormats.add(fmt.format);
        }
    }

    int total = allFiles.size();
    for (int i = 0; i < total; ++i) {
        juce::OwnedArray<juce::PluginDescription> descriptions;
        try {
            fileFormats[i]->findAllTypesForFile(descriptions, allFiles[i]);
        } catch (...) {
            qWarning() << "[PluginScanner] plugin crashed during scan:"
                        << allFiles[i].toRawUTF8();
            continue;
        }

        for (auto* desc : descriptions) {
            emit pluginFound(*desc);
            emit scanProgress(QString::fromStdString(desc->name.toStdString()),
                              i + 1, total);
        }
    }

    emit scanFinished();
}

PluginScanner::PluginScanner(AudioEngine& engine, QObject* parent)
    : QObject(parent), audioEngine_(engine)
{
    loadCachedList();
}

PluginScanner::~PluginScanner()
{
    workerThread_.quit();
    workerThread_.wait();
}

void PluginScanner::startScan()
{
    if (scanning_)
        return;

    if (workerThread_.isRunning()) {
        workerThread_.quit();
        workerThread_.wait();
    }

    scanning_ = true;

    auto* worker = new PluginScanWorker(audioEngine_.engine());
    worker->moveToThread(&workerThread_);

    connect(&workerThread_, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &PluginScanWorker::scanProgress,
            this, &PluginScanner::scanProgress);
    connect(worker, &PluginScanWorker::pluginFound, this,
            [this](const juce::PluginDescription& desc) {
                getPluginList().addType(desc);
            }, Qt::QueuedConnection);
    connect(worker, &PluginScanWorker::scanFinished, this, [this]() {
        scanning_ = false;
        saveCachedList();
        emit scanFinished();
    });

    workerThread_.start();
    QMetaObject::invokeMethod(worker, &PluginScanWorker::doScan);
}

juce::KnownPluginList& PluginScanner::getPluginList()
{
    return audioEngine_.engine().getPluginManager().knownPluginList;
}

juce::File PluginScanner::getCacheFile() const
{
    auto appData = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);
    return appData.getChildFile("OpenDaw").getChildFile("plugin-cache.xml");
}

void PluginScanner::loadCachedList()
{
    auto cacheFile = getCacheFile();
    if (!cacheFile.existsAsFile()) return;

    auto xml = juce::XmlDocument::parse(cacheFile);
    if (!xml) {
        qWarning() << "[PluginScanner] corrupt plugin cache, deleting";
        cacheFile.deleteFile();
        return;
    }
    getPluginList().recreateFromXml(*xml);

    auto& kpl = getPluginList();
    for (int i = kpl.getNumTypes() - 1; i >= 0; --i) {
        auto desc = kpl.getTypes()[i];
        if (!juce::File(desc.fileOrIdentifier).existsAsFile())
            kpl.removeType(desc);
    }
}

void PluginScanner::saveCachedList()
{
    auto cacheFile = getCacheFile();
    cacheFile.getParentDirectory().createDirectory();

    if (auto xml = getPluginList().createXml()) {
        xml->writeTo(cacheFile);
    }
}

} // namespace OpenDaw
