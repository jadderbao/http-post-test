#include "http_client.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include <QHttpMultiPart>
#include <QEventLoop>

//http_request_body

http_request_body::http_request_body(const QByteArray& data)
	: _body_type(REQUEST_BODY_TYPE_DATA)
	, _body_data(new QByteArray(data))
{
}

http_request_body::http_request_body(QHttpMultiPart *multi_part)
	: _body_type(REQUEST_BODY_TYPE_MULTI_PART)
	, _body_data(multi_part)
{
}

http_request_body::http_request_body(QIODevice *io_device)
    : _body_type(REQUEST_BODY_TYPE_IO_DEVICE)
    , _body_data(io_device)
{

}

http_request_body::~http_request_body()
{
	if (_body_type == REQUEST_BODY_TYPE_DATA && _body_data.data){
		delete _body_data.data;
	}

	if (_body_type == REQUEST_BODY_TYPE_MULTI_PART && _body_data.multi_part){
		delete _body_data.multi_part;
	}
}


http_request_body::request_body_type_t http_request_body::type()
{
	return _body_type;
}

const QByteArray http_request_body::data() const
{
	return *_body_data.data;
}

QHttpMultiPart * http_request_body::multi_part()
{
    return _body_data.multi_part;
}

QIODevice *http_request_body::io_device()
{
    return _body_data.io_device;
}


//http_request

http_request::http_request(const QUrl& url, QObject *parent)
	:QObject(parent)
	, _request(new QNetworkRequest(url))
	, _body(0)
{
	QSslConfiguration config;
	config.setPeerVerifyMode(QSslSocket::VerifyNone);
	config.setProtocol(QSsl::TlsV1_0OrLater);
	_request->setSslConfiguration(config);
}

http_request::~http_request()
{
	delete _request;
	delete _body;
}

QNetworkRequest * http_request::request() const
{
	return _request;
}

void http_request::set_url(const QUrl& url)
{
	_request->setUrl(url);
}

void http_request::set_body(http_request_body *body)
{
	_body = body;
}

http_request_body * http_request::body() const
{
	return _body;
}

//http_client

http_client::http_client(QNetworkAccessManager *am, QObject *parent)
	: QObject(parent)
	, _am(am)
{

}

http_client::~http_client()
{

}

bool http_client::process_and_check_redirect(bool redirect, QNetworkReply *reply, QNetworkRequest& req)
{
    QEventLoop loop;
    connect(reply, &QNetworkReply::downloadProgress, this, &http_client::downloadProgress);
    connect(reply, &QNetworkReply::uploadProgress, this, &http_client::uploadProgress);
    connect(reply, &QNetworkReply::finished, this, &http_client::finished);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (redirect &&
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 302){
        req.setUrl(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString());
        reply->close();
        reply->deleteLater();
        return true;
    }

    return false;
}

QNetworkReply * http_client::post(http_request *request, bool redirect /*= true*/)
{
	QNetworkReply *reply = 0;

	while (true){

		QNetworkRequest& req = *request->request();
		http_request_body *body = request->body();
        if(!body){
            reply = _am->post(req, QByteArray());
        }else{
            switch (body->type())
            {
            case http_request_body::REQUEST_BODY_TYPE_DATA:
                reply = _am->post(req, body->data());
                break;
            case http_request_body::REQUEST_BODY_TYPE_MULTI_PART:
                reply = _am->post(req, body->multi_part());
                break;
            case http_request_body::REQUEST_BODY_TYPE_IO_DEVICE:
                reply = _am->post(req, body->io_device());
                break;
            default:
                break;
            }
        }

        if(reply && process_and_check_redirect(redirect, reply, req)){
            continue;
        }

        if(body && body->type() == http_request_body::REQUEST_BODY_TYPE_IO_DEVICE
            && body->io_device()){
            body->io_device()->close();
            body->io_device()->deleteLater();
        }

		break;
	}

	return reply;
}

QNetworkReply *http_client::get(http_request *request, bool redirect /*= true*/)
{
	QNetworkReply *reply = 0;
	while (true){

		QNetworkRequest& req = *request->request();

		reply = _am->get(req);
        if(reply && process_and_check_redirect(redirect, reply, req)){
            continue;
        }

		break;
	}

    return reply;
}

QNetworkReply *http_client::customRequest(http_request *request, QString &verb, bool redirect)
{
    QNetworkReply *reply = 0;

    while (true){

        QNetworkRequest& req = *request->request();
        http_request_body *body = request->body();

        if(!body){
            reply = _am->sendCustomRequest(req, verb.toLocal8Bit());
        }else{
            switch (body->type())
            {
            case http_request_body::REQUEST_BODY_TYPE_DATA:
                reply = _am->sendCustomRequest(req, verb.toLocal8Bit(), body->data());
                break;
            case http_request_body::REQUEST_BODY_TYPE_MULTI_PART:
                reply = _am->sendCustomRequest(req, verb.toLocal8Bit(), body->multi_part());
                break;
            case http_request_body::REQUEST_BODY_TYPE_IO_DEVICE:
                reply = _am->sendCustomRequest(req, verb.toLocal8Bit(), body->io_device());
                break;
            default:
                break;
            }
        }

        if(reply && process_and_check_redirect(redirect, reply, req)){
            continue;
        }

        if(body && body->type() == http_request_body::REQUEST_BODY_TYPE_IO_DEVICE
            && body->io_device()){
            body->io_device()->close();
            body->io_device()->deleteLater();
        }

        break;
    }

    return reply;
}

