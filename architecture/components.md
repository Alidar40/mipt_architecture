# Компонентная архитектура
<!-- Состав и взаимосвязи компонентов системы между собой и внешними системами с указанием протоколов, ключевые технологии, используемые для реализации компонентов.
Диаграмма контейнеров C4 и текстовое описание. 
-->
## Компонентная диаграмма

```plantuml
@startuml
!include https://raw.githubusercontent.com/plantuml-stdlib/C4-PlantUML/master/C4_Container.puml

AddElementTag("microService", $shape=EightSidedShape(), $bgColor="CornflowerBlue", $fontColor="white", $legendText="microservice")
AddElementTag("storage", $shape=RoundedBoxShape(), $bgColor="lightSkyBlue", $fontColor="white")

Person(admin, "Администратор")
Person(shop_worker, "Сотрудник магазина")
Person(user, "Пользователь")

System_Ext(shopping_item, "Товар")
System_Ext(shopping_cart, "Корзина")

System_Boundary(shop, "Магазин") {
   Container(app, "Клиентское веб-приложение", "html, JavaScript, Angular", "Сайт магазина")
   Container(client_service, "Сервис авторизации", "C++", "Сервис управления пользователями", $tags = "microService")    
   Container(shopping_item_service, "Сервис товаров", "C++", "Сервис управления информацией о товарах", $tags = "microService")       
   Container(shopping_cart_service, "Сервис корзин", "C++", "Сервис управления информацией о корзинах", $tags = "microService") 
   ContainerDb(db, "Данные магазина", "MySQL", "Хранение данных магазина", $tags = "storage")
}

Rel_D(admin, shop, "Добавление/просмотр информации о пользователях")
Rel_D(shop_worker, shop, "Добавление, просмотр информации о товарах")
Rel_D(user, shop, "Добавление товаров в корзину, удаление товаров из корзины, просмотр информации о корзине, оформление покупки")

Rel(app, client_service, "Создание пользователя", "http://localhost/person")
Rel(client_service, db, "INTSERT/SELECT/UPDATE", "SQL")

Rel(app,shopping_item_service, "Работа с товарами", "http://localhost/shoppping_item")
Rel_L(shopping_item_service, shopping_item, "Запрос информации о товарах")
Rel(shopping_item_service, db, "INTSERT/SELECT/UPDATE", "SQL")

Rel(app,shopping_cart_service, "Работа с корзинами", "http://localhost/shopping_cart")
Rel_R(shopping_cart_service, shopping_cart, "Запрос текущей корзины")
Rel(shopping_cart_service, db, "INTSERT/SELECT/UPDATE", "SQL")


@enduml
```
## Список компонентов
### Сервис авторизации


**API**:
-	Создание нового пользователя
      - входыне параметры: login, Имя, Фамилия, email, обращение (гн/гжа)
      - выходные параметры, отсутствуют
-	Поиск пользователя по логину
     - входные параметры:  login
     - выходные параметры: Имя, Фамилия, email, обращение (гн/гжа)
-	Поиск пользователя по маске имя и фамилии
     - входные параметры: маска фамилии, маска имени
     - выходные параметры: login, Имя, Фамилия, email, обращение (гн/гжа)

### Сервис товаров
**API**:
- Создание товара
  - Входные параметры: название, цена
  - Выходыне параметры: отсутствуют
- Получение списка товаров
  - Входные параметры: диапазон цен
  - Выходные параметры: массив пар название - цена


### Сервис корзин
**API**:
- Добавление товара в корзину
  - Входные параметры:  название товара
  - Выходные параметры: отсутствуют
- Получение корзины пользователя
  - Входные параметры: login пользователя
  - Выходные параметры: массив пар название товара - цена
