#include "serializer.hpp"
#include "converter.hpp"

#include <QDebug>
#include <QFile>
#include <QSettings>

cs::Serializer::Serializer(const QString& fileName, QObject* parent):
    QObject(parent)
{
    bool exists = QFile::exists(fileName);
    settings = std::make_unique<QSettings>(fileName, QSettings::Format::IniFormat);

    if (!exists) {
        writeDefaultData();
    }
}

cs::Data cs::Serializer::readData() const
{
    cs::Data data;

    settings->beginGroup(cs::Literals::paramsKey);

    if (settings->contains(cs::Literals::nodeTypeParameter)) {
        data.nodeType = settings->value(cs::Literals::nodeTypeParameter).value<QString>();
    }

    if (settings->contains(cs::Literals::boostrapTypeParameter)) {
        data.boostrapType = settings->value(cs::Literals::boostrapTypeParameter).value<QString>();
    }

    if (settings->contains(cs::Literals::ipv6TypeParameter)) {
        data.isIpv6 = settings->value(cs::Literals::ipv6TypeParameter).value<bool>();
    }

    settings->endGroup();
    settings->beginGroup(cs::Literals::signalServerKey);

    if (settings->contains(cs::Literals::ipParameter)) {
        data.signalServerIp = settings->value(cs::Literals::ipParameter).value<QString>();
    }

    if (settings->contains(cs::Literals::portParameter)) {
        data.signalServerPort = settings->value(cs::Literals::portParameter).value<int>();
    }

    settings->endGroup();
    settings->beginGroup(cs::Literals::hostInputKey);

    if (settings->contains(cs::Literals::portParameter)) {
        data.nodeInputPort = settings->value(cs::Literals::portParameter).value<int>();
    }

    settings->endGroup();
    settings->beginGroup(cs::Literals::hostOutputKey);

    if (settings->contains(cs::Literals::ipParameter)) {
        data.nodeIp = settings->value(cs::Literals::ipParameter).value<QString>();
    }

    if (settings->contains(cs::Literals::portParameter)) {
        data.nodeOutputPort = settings->value(cs::Literals::portParameter).value<int>();
    }

    settings->endGroup();
    settings->beginGroup(cs::Literals::apiKey);

    if (settings->contains(cs::Literals::apiExecutorPortParameter)) {
        data.apiExecutorPort = settings->value(cs::Literals::apiExecutorPortParameter).value<int>();
    }

    if (settings->contains(cs::Literals::executorPortParameter)) {
        data.executorPort = settings->value(cs::Literals::executorPortParameter).value<int>();
    }

    if (settings->contains(cs::Literals::ajaxPortParameter)) {
        data.ajaxPort = settings->value(cs::Literals::ajaxPortParameter).value<int>();
    }

    if (settings->contains(cs::Literals::apiPortParameter)) {
        data.apiPort = settings->value(cs::Literals::apiPortParameter).value<int>();
    }

    settings->endGroup();

    return data;
}

void cs::Serializer::writeData(const Data& data)
{
    // TODO: fix when it will be used on ui
    int ajaxPort = settings->value(cs::Literals::apiKey + QString("/") + cs::Literals::ajaxPortParameter).toInt();
    ajaxPort = ajaxPort ? ajaxPort : data.ajaxPort;
    ajaxPort = ajaxPort ? ajaxPort : cs::defaultAjaxPort;

    clear();

    // params
    settings->beginGroup(cs::Literals::paramsKey);

    if (!data.nodeType.isEmpty()) {
        settings->setValue(cs::Literals::nodeTypeParameter, data.nodeType);
    }

    if (!data.boostrapType.isEmpty()) {
        settings->setValue(cs::Literals::boostrapTypeParameter, data.boostrapType);
    }

    settings->setValue(cs::Literals::ipv6TypeParameter, data.isIpv6);
    settings->endGroup();

    // signal_server
    settings->beginGroup(cs::Literals::signalServerKey);

    if (!data.signalServerIp.isEmpty() && data.signalServerPort) {
        settings->setValue(cs::Literals::ipParameter, data.signalServerIp);
        settings->setValue(cs::Literals::portParameter, data.signalServerPort);
    }

    settings->endGroup();

    // host_input
    const QString hostInputKey = cs::Literals::hostInputKey + QString("/") + cs::Literals::portParameter;
    const int hostInputPort = data.nodeInputPort ? data.nodeInputPort : cs::defaultHostInputPort;
    settings->setValue(hostInputKey, hostInputPort);

    // host_output
    settings->beginGroup(cs::Literals::hostOutputKey);

    if (data.nodeOutputPort) {
        settings->setValue(cs::Literals::portParameter, data.nodeOutputPort);

        if (!data.nodeIp.isEmpty()) {
            settings->setValue(cs::Literals::ipParameter, data.nodeIp);
        }
    }

    settings->endGroup();

    // api
    settings->beginGroup(cs::Literals::apiKey);

    settings->setValue(cs::Literals::apiExecutorPortParameter, data.apiExecutorPort);
    settings->setValue(cs::Literals::executorPortParameter, data.executorPort);
    settings->setValue(cs::Literals::apiPortParameter, data.apiPort);
    settings->setValue(cs::Literals::ajaxPortParameter, ajaxPort);

    settings->endGroup();

    emit writeCompleted();
}

void cs::Serializer::clear()
{
    settings->remove(cs::Literals::signalServerKey);
    settings->remove(cs::Literals::apiKey);
    settings->remove(cs::Literals::hostOutputKey);
}

void cs::Serializer::writeDefaultData()
{
    cs::Data data;
    data.nodeType = cs::Literals::clientType;
    data.boostrapType = cs::Literals::signalServerType;
    data.isIpv6 = false;
    data.nodeInputPort = cs::defaultHostInputPort;
    data.signalServerIp = cs::defaultSignalServerIp;
    data.signalServerPort = cs::defaultSignalServerPort;

    data.apiExecutorPort = cs::defaultApiExecutorPort;
    data.executorPort = cs::defautlExecutorPort;
    data.apiPort = cs::defaultApiPort;
    data.ajaxPort = cs::defaultAjaxPort;

    writeData(data);

    // hosts
    settings->beginGroup(cs::Literals::paramsKey);
    settings->setValue("hosts_filename", cs::Literals::hostsFileName);
    settings->endGroup();

    // sinks.default
    settings->beginGroup(cs::Literals::sinksDefaultKey);

    settings->setValue("Destination", R"("Console")");
    settings->setValue("Filter", "%Severity% >= info");
    settings->setValue("Format", R"("[%Severity%] %Message%")");

    settings->endGroup();

    convert();
}

void cs::Serializer::convert()
{
    QString fileName;

    if (settings) {
        fileName = settings->fileName();
        settings.reset();
    }

    fileName = fileName.isEmpty() ? cs::Literals::configFileName : fileName;

    if (!cs::Converter::convert(fileName)) {
        qDebug() << "Convert error occured";
    }

    settings = std::make_unique<QSettings>(fileName, QSettings::IniFormat);
}

cs::Serializer::~Serializer() = default;
