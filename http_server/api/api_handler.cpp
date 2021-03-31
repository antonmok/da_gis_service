#include <map>
#include <functional>
#include <iostream>

#include "url/url.hpp"
#include "jwt/json_web_tokens.h"
#include "api_handler.h"

bool GetParamKey(const std::string& path, unsigned idx, std::string& key_val);

bool LogoutUser(std::string&);
bool SecretTest(std::string&);
bool InfoUser(std::string&);
bool Settings(std::string&);
bool SettingsMobile(std::string&);

CAPIHandler& CAPIHandler::Instance()
{
	static CAPIHandler singletonInstance;
	return singletonInstance;
}

CAPIHandler::CAPIHandler()
{
    //route_methods_["api/login"] = LoginUser;
    route_methods_["/api/logout"] = LogoutUser;
    route_methods_["/api/secret"] = SecretTest;
    route_methods_["/api/info"] = InfoUser;
    route_methods_["/api/settings"] = Settings;
    route_methods_["/api/settings/mobile"] = SettingsMobile;
    /*route_methods_["/api/settings"] = ;
    route_methods_["/api/settings"] = ;
    route_methods_["/api/settings"] = ;
    route_methods_["/api/settings"] = ;
    route_methods_["/api/"] = ;
    route_methods_["/api/"] = ;
    route_methods_["/api/"] = ;
    route_methods_["/api/"] = ;
    route_methods_["/api/"] = ;
    route_methods_["/api/"] = ;
    route_methods_["/api/"] = ;
    route_methods_["/api/"] = ;
    route_methods_["/api/"] = ;
    route_methods_["/api/"] = ;
    route_methods_["/api/"] = ;
    route_methods_["/api/"] = ;
    route_methods_["/api/"] = ;
    route_methods_["/api/"] = ;
    route_methods_["/api/"] = ;*/

}

CAPIHandler::~CAPIHandler() {}

/********* API METHODS ********/

http::status CAPIHandler::LoginUser(const std::string& body, std::string& response) const
{
    // parse body json
    Credentials creds;
    if (JsonToStruct(body, creds)) {

        std::string token;
        if (CUsersData::Instance().AuthenticateUser(creds, token)) {
            // form response
            response = StructToJson(JSToken{ token });
            return http::status::ok;
        }
    } else {
        response = "Invalid json";
        return http::status::bad_request;
    }

    return http::status::unauthorized;
}

bool LogoutUser(std::string& response)
{
    CUsersData::Instance().DisproveUser(CAPIHandler::Instance().GetRequestContext().token);

    return true;
}

bool SecretTest(std::string& response)
{
    // parse body as json
    /*json::error_code ec;
    json::value js_body = json::parse(body, ec);

    if (ec) {
        resp_str.assign(ec.message());
        return false;
    }*/

    response.assign("secret method called");
    return true;
}

bool InfoUser(std::string& response)
{
    JSUserInfo js_user;
    CUsersData::UserData user_data = CUsersData::Instance().GetUserData(CAPIHandler::Instance().GetRequestContext().token);

    js_user.firstLogin = user_data.firstLogin;
    js_user.isAdmin = user_data.isAdmin;
    js_user.isUser = user_data.isUser;
    js_user.name = user_data.name;
    js_user.position = user_data.position;

    response = StructToJson(js_user);

    return true;
}

bool Settings(std::string& response)
{
    
}

bool SettingsMobile(std::string& response)
{
    
}

/*


route: 'api/settings/mobile/operators',
method: GET,
response: {
  operators: [
    {
      id: operatorId,
      title: operatorTitle
    }
  ]
},
description: Запрос списка мобильных операторов.

route: 'api/settings/mobile/operators',
method: POST,
body: {
  operator: operatorId
},
response: {
  success,
  operator: {
    id: operatorId,
    title: operatorTitle
  }
},
description: команда на переключение мобильного интернета. Передаю id оператора, выключить/включить. В ответ жду сообщение о успешности и id и title нового оператора.

### Уведомления

route: 'api/settings/notifications',
method: POST,
body: {
  type: grs,
  sounds: {
    isOn: true/false,
    turnOff: true/false,
    turnOn: true/false
  },
  banners: {
    isOn: true/false,
    turnOff: true/false,
    turnOn: true/false
  }
},
response: {
  success,
  type: grs,
  sounds: {
    isOn: true/false
  },
  banners: {
    isOn: true/false
  }
},
description: команда на включение/выключение оповещений. Передаю тип оповещения (совпадение ГРЗ например) текущее состояние и команду, выключить/включить. В ответ жду сообщение о успешности и новое состояние.

### Звуки

route: 'api/settings/sounds',
method: POST,
body: {
  type: notifications,
  sound: soundId
},
response: {
  success,
  type: notifications,
  sounds: soundId,
},
description: команда на сохранение звука оповещения. Передаю тип звука (оповещения) и выбранный звук. В ответ жду сообщение о успешности и новое состояние.




## Инфо по комплексу

route: 'api/statistics',
method: GET,
response: {
  connections: {
    traffic: '11mB',
    wifi: 3,
    gps: true/false, ?
    mobile: 3,
  },
  statistics: {
    startTime: timestamp,
    vehiclesTotal: 1785,
    foulsTotal: 206,
    sended: 150,
    queue: 56
  }

}
description: информация о соединениях, статистика работы комплекса, может быть что-то еще. traffic: объем переданных данных? wifi: сила сигнала (от 1 до 4). gps: подключен или нет ? mobile: сила сигнала (от 0 до 3). startTime: timestamp времени начала работы комплекса, vehiclesTotal: всего зафиксировано ТС, foulsTotal: всего зафиксировано нарушений. sended: отправлено фотоматериалов. queue: в очереди на отправку


## Карта
route: 'api/map,
method: GET,
response: {
  position: [59.912220, 30.317482],
  addres: 'addres'
}
description: position: массив с координатами местоположения, addres: адрес местоположения (улица)

## Инфо
route: 'api/mode-info,
method: GET,
response: {
  speedLimit: 60,
  edge: 81
}
description: speedLimit: разрешенная скорость, adge: порог фиксации

## Зафиксированные события
route: 'api/mode-events,
method: GET,
response: {
  events: [
    {
      id: eventId,
      img: base64 image,
      grs: 'c243oe178',
      speed: 90,
      time: timestamp
    }
  ]
}
description: events: массив зафиксированных событий в этой сессии, id: id события, img: фото в base64, grs: ГРЗ, speed: скорость, time: timestamp события

## Событие
route: 'api/event,
method: GET,
params: {
  id: eventId
}
response: {
  event: {
    id: eventId,
    img: base64 image,
    grs: 'c243oe178',
    speed: 90,
    time: timestamp,
    информация для страницы просмотра зафиксированного нарушения
  }
}
description: передаю id события. Хочу получить: event: объект зафиксированного события, id: id события, img: фото в base64, grs: ГРЗ, speed: скорость, time: timestamp события, и тд. (пока не понял по фигме, что конкретно там отображать и как)


## Настройки режимов

### Сохранение настроек режимов

#### Автоматический
route: 'api/settings/auto',
method: POST,
body: {
  fines: ['12.5', '12.9']
},
response: 'success' или 'ok',
description: запись настроек автоматического режима. Если успешно, ожидаю получить подтверждение "ок".

#### Автоматизированный
route: 'api/settings/automated',
method: POST,
body: {
  speedLimit: 42,
  maxFines: 2,
  period: 1,
  pause: 3
},
response: 'success' или 'ok',
description: запись настроек автоматизированного режима. Если успешно, ожидаю получить подтверждение "ок".

#### Комбинированный
route: 'api/settings/combi',
method: POST,
body: {
  speedLimit: 42,
  maxFines: 2,
  period: 1,
  pause: 3
},
response: 'success' или 'ok',
description: запись настроек комбинированного режима. Если успешно, ожидаю получить подтверждение "ок".

#### Комбинированный
route: 'api/settings/delay-rec',
method: POST,
body: {
  duration: 2,
},
response: 'success' или 'ok',
description: запись настроек отложенной записи. Если успешно, ожидаю получить подтверждение "ок".

### Сохранение наборов настроек

route: 'api/settings/modesSettingsPacks',
method: POST,
body: {
  index: 2,
},
response: 'success' или 'ok',
description: запись набора настроек всех режимов. Сохраняет в набор настроек все текущие настройки режимов. Буду передавать порядковый индекс набора настроек для сверки фронта и бека. Если успешно, ожидаю получить подтверждение "ок". Если достигнуто максимальное число наборов настроек, то буду проверять это на фронте, но, наверное, и на беке лучше тоже проверять и если что возвращать ошибку 409.

* Также в фигме этого нет, но, думаю, нужно предусмотреть и удаление наборов настроек. В ресте есть метод delete.

route: 'api/settings/modesSettingsPacks',
method: DELETE,
body: {
  index: 3,
},
response: 'success' или 'ok',
description: Удаляет выбранный набор настроек по его порядковому индексу. Если успешно, ожидаю получить подтверждение "ок".


## Поиск по ГРЗ
route: 'api/settings/search-grs',
method: GET,
params: {
  number: 'x111xx111',
},
response: {
  results: [
    {
      id: id,
      grs: 'x111xx111'
    },
    {
      id: id,
      grs: 'x111xx111'
    }
  ]
},
description: Хочу получить массив результатов поиска.

## Команды
route: 'api/shortcommand',
method: post,
body: {
  type: speed/park/trajectory
  on: true/false,
},
response: {
  success
},
description: Передаю тип комманды и состояние, в которое нужно перевести.

route: 'api/mode',
method: post,
body: {
  mode: idMode/titleMode
  on: true/false,
},
response: {
  success
},
description: Передаю тип режима и состояние, в которое нужно перевести.

route: 'api/cams',
method: post,
body: {
  cams: [idCam1, idCam2]
},
response: {
  success
},
description: Передаю массив, в котором передаю айдишники выбранных камер.
*/
/*************************************/

bool GetParamKey(const std::string& path, unsigned idx, std::string& key_val)
{
    // parse url, get parameters
    Url url(path);
    //auto sz = url.query().size();
    key_val.assign(url.query(idx).key());

    return true;
}

bool CAPIHandler::ExecuteRouteMethod(std::string& response) const
{
    return route_methods_.at(request_context_.route)(response);
}

void CAPIHandler::GetRequestContext(CAPIHandler::RequestContext& ctx) const
{
    ctx = request_context_;
}

const CAPIHandler::RequestContext& CAPIHandler::GetRequestContext() const
{
    return request_context_;
}
