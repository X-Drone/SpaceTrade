#include <iostream>
#include <cmath>
#include <windows.h>
#include <vector>
#include <ctime>
#include <unordered_map>

using namespace std;

bool KEY[256];

void GetKEY()
{
    int i = 0;
    while (i < 256)
    {
        if (GetAsyncKeyState(i)) KEY[i] = 1; else KEY[i] = 0;
        i++;
    }
}

class Point
{
private:
    int x, y;
public:
    Point()
    {
        x = 0, y = 0;
    }
    Point(int _x, int _y)
    {
        x = _x, y = _y;
    }
    int getX()
    {
        return x;
    }
    int getY()
    {
        return y;
    }
    double length(Point _point)
    {
        return sqrt(pow(_point.getX() - x, 2) + pow(_point.getY() - y, 2));
    }
};

class ConsoleColor {
public:
    static COLORREF red() { return RGB(255, 0, 0); }
    static COLORREF green() { return RGB(0, 255, 0); }
    static COLORREF blue() { return RGB(0, 0, 255); }
    static COLORREF yellow() { return RGB(255, 255, 0); }
    static COLORREF white() { return RGB(255, 255, 255); }
    static COLORREF black() { return RGB(0, 0, 0); }
    static COLORREF portal() { return RGB(0, 128, 255); }
    static COLORREF random_color() { return RGB((rand() % 256), (rand() % 256), (rand() % 256)); }
};

class Circle
{
private:
    Point center;
    int radius;
    COLORREF color;
public:
    Circle(Point _center, int _radius, COLORREF _color)
    {
        center = _center, radius = _radius;
        color = _color;
    }
    bool isInside(Point _point)
    {
        return center.length(_point) < radius;
    }
    int getRadius()
    {
        return radius;
    }
    Point getCenter()
    {
        return center;
    }
    COLORREF getColor()
    {
        return color;
    }
    void setColor(COLORREF _color)
    {
        color = _color;
        ConsoleColor::random_color();
    }
};

class ConsoleDrawer {
    HANDLE _conHandle;
    HWND _hwnd;
    HDC _hdc;
    PAINTSTRUCT _ps;
    unordered_map<COLORREF, HGDIOBJ> _bm;
    unordered_map<COLORREF, HGDIOBJ> _pm;
    HGDIOBJ _curentBrush;
    HGDIOBJ _curentPen;


    int _width;
    int _height;
    bool _isWork = true;

    COLORREF getColor(int r, int g, int b) {
        return RGB(r, g, b);
    }

    void selectBrush(COLORREF color, bool filled = false) {
        if (!filled) {
            SelectObject(_hdc, CreateSolidBrush(NULL_BRUSH));
            return;
        }
        if (_bm.find(color) == _bm.end())
            _bm.insert({ color, CreateSolidBrush(color) });

        if (_curentBrush != _bm[color]) {
            _curentBrush = _bm[color];
            SelectObject(_hdc, _curentBrush);
        }
    }

    void selectPen(COLORREF color) {
        if (_pm.find(color) == _pm.end())
            _pm.insert({ color, CreatePen(PS_SOLID, 1, color) });
        if (_curentPen != _pm[color]) {
            _curentPen = _pm[color];
            SelectObject(_hdc, _curentPen);
        }
    }

public:
    ConsoleDrawer() {
        setlocale(LC_ALL, "");
        _conHandle = GetStdHandle(STD_INPUT_HANDLE);
        int t = sizeof(HGDIOBJ);
        _hwnd = GetConsoleWindow();
        RECT rc;
        GetClientRect(_hwnd, &rc);
        _width = rc.right;
        _height = rc.bottom;
        _hdc = GetDC(_hwnd);
    }

    void clearScreen() {
        system("cls");
    }

    int getWidth() { return _width; }
    int getHeight() { return _height; }
    void setWidth(int width) { _width = width; }
    void setHeight(int height) { _height = height; }

    void processLoop(int time) {
        Sleep(time);

        KEY_EVENT_RECORD key;
        INPUT_RECORD irec[100];
        DWORD cc;
        PeekConsoleInput(_conHandle, irec, 100, &cc);
        for (DWORD i = 0; i < 100; ++i) {
            if (irec[i].EventType == KEY_EVENT && ((KEY_EVENT_RECORD&)irec[i].Event).bKeyDown) {
                key = (KEY_EVENT_RECORD&)irec[i].Event;
                if (key.wVirtualKeyCode == VK_ESCAPE)
                    _isWork = false;
            }
        }
    }

    void drawBackground(COLORREF color = 0) {
        RECT rc;
        GetClientRect(_hwnd, &rc);
        drawRect(Point(), Point(_width, _height), color, true);
    }

    bool isWork() {

        return _isWork;
    }

    void drawPoint(Point p, COLORREF color) {
        SetPixel(_hdc, p.getX(), p.getY(), color);
    }

    void drawLine(Point p1, Point p2, COLORREF color) {
        selectPen(color);
        //SelectObject(_hdc, CreatePen(PS_SOLID, 1, color));
        //SetDCPenColor(_hdc, color);
        MoveToEx(_hdc, p1.getX(), p1.getY(), NULL);
        LineTo(_hdc, p2.getX(), p2.getY());
    }

    void drawCircle(Point c, int radius, COLORREF color, bool filled = false) {
        selectPen(color);
        selectBrush(color, filled);
        //SetDCBrushColor(_hdc, color);
        Ellipse(_hdc, c.getX() - radius, c.getY() - radius, c.getX() + radius, c.getY() + radius);

    }

    // Рисует прямоугольник по двум точкам
    // p1 - верхний левый угол
    // p2 - нижний правый угол
    void drawRect(Point p1, Point p2, COLORREF color, bool filled = false) {
        selectPen(color);
        selectBrush(color, filled);
        Rectangle(_hdc, p1.getX(), p1.getY(), p2.getX(), p2.getY());
    }

    // Рисует треугольник по трем точкам
    void drawTriangle(Point p1, Point p2, Point p3, COLORREF color, bool filled = false) {
        selectPen(color);
        selectBrush(color, filled);
        POINT apt[3];
        apt[0].x = p1.getX(); apt[0].y = p1.getY();
        apt[1].x = p2.getX(); apt[1].y = p2.getY();
        apt[2].x = p3.getX(); apt[2].y = p3.getY();
        Polygon(_hdc, apt, 3);
    }

    //рисует четырёхугольник по 4 точкам
    void drawfourangle(Point p1, Point p2, Point p3, Point p4, COLORREF color, bool filled = false)
    {
        selectPen(color);
        selectBrush(color, filled);
        POINT apt[4];
        apt[0].x = p1.getX(); apt[0].y = p1.getY();
        apt[1].x = p2.getX(); apt[1].y = p2.getY();
        apt[2].x = p3.getX(); apt[2].y = p3.getY();
        apt[3].x = p4.getX(); apt[3].y = p4.getY();
        Polygon(_hdc, apt, 4);
    }
};

ConsoleDrawer cd;

class CreditAccount {
private:
    int credits = 0;
public:
    int getCredits()
    {
        return credits;
    }
    void setCredits(int _credits)
    {
        credits = _credits;
    }
    void add(int _credits, string command)
    {
        if (command == "credits")
            credits += _credits;
        else if (command == "bools")
            credits += (_credits * 3);
        else if (command == "stars")
            credits += (_credits * 2 / 5);
    }
    void buy(int _credits, string command)
    {
        if (command == "credits")
            credits -= _credits;
        else if (command == "bools")
            credits -= (_credits * 3);
        else if (command == "stars")
            credits -= (_credits * 2 / 5);
    }
    void printStatus()
    {
        cout << "Account: " << credits << " credits " << endl;
    }
};

class Product
{
private:
    int count;
    int price;
    string name;
public:
    Product(string _name = "random", int _price = 250, int _count = 1)
    {
        count = _count;
        price = _price;
        name = _name;
    }
    int getCount()
    {
        return count;
    }
    int getPrice()
    {
        return price;
    }
    string getName()
    {
        return name;
    }
    void setCount(int _count)
    {
        count = _count;
    }
    void setPrice(int _price)
    {
        price = _price;
    }
    void setName(string _name)
    {
        name = _name;
    }
};

vector <string> products
{
    "react_fuel", "details", "computer_plates", "bio_material", "computer_modules"
};
vector <string> statNames
{
    "Orion", "PTNI", "Omicron", "Babilon", "Seyzwel", "Ianamus Zura", "Epsilon", "Xinelon", "Taurenium", "Atenran"
};

vector <string> shipTypes
{
    "Nova", "Nemesida", "Hunter", "Thrillar", "Tor", "Conqueror"
};
vector <string> shipNames
{
    ""
};

class Box
{
private:
    int envMax = 50;
    int envLeft = envMax;
public:
    vector <Product> products;

    int getEnvMax()
    {
        return envMax;
    }
    int getEnvLeft()
    {
        return envLeft;
    }
    void setEnvMax(int _envMax)
    {
        envMax = _envMax;
    }
    void setEnvLeft(int _envLeft)
    {
        envLeft = _envLeft;
    }
    void addCargo(Product p)
    {
        if (p.getCount() + envLeft <= envMax)
        {

            products.push_back(p);
            envLeft += p.getCount();
        }
    }
    void delCargo(Product p)
    {
        for (int i = 0; i < products.size(); i++)
            if ((p.getName() == products[i].getName()) && (p.getCount() <= products[i].getCount()))
            {
                if (p.getCount() < products[i].getCount())
                    products[i].setCount(products[i].getCount() - p.getCount());
                else
                {
                    for (int j = i; j < products.size(); j++)
                        products[j] = products[j + 1];
                    products.pop_back();
                }
            }
    }
};

class SpacePoint
{
private:
    int x;
    int y;
public:
    SpacePoint()
    {
        x = 0;
        y = 0;
    }
    SpacePoint(int _x, int _y)
    {
        x = _x;
        y = _y;
    }
    int getX()
    {
        return x;
    }
    int getY()
    {
        return y;
    }
    void setX(int _x)
    {
        x = _x;
    }
    void setY(int _y)
    {
        y = _y;
    }
    int distance(SpacePoint _point)
    {
        return sqrt(pow(x - _point.x, 2) + pow(y - _point.y, 2));
    }
};

class SpaceShip
{
protected:
    int fuelMax = 10000;
    int fuelLeft;
    int gunReact = 100;
    int gunAngle;
    string name;
    SpacePoint shipPoint;
    CreditAccount accaunt205;
public:
    Box cargo;
    bool isShoot;
    SpacePoint shootPoint;

    SpaceShip(string _name = "Your_ship", int _credits = 5000)
    {
        accaunt205.setCredits(_credits);
        cargo.setEnvMax(250);
    }
    int getFuelLeft()
    {
        return fuelLeft;
    }
    int getFuelMax()
    {
        return fuelMax;
    }
    string getName()
    {
        return name;
    }
    void setGunReact(int _gunReact)
    {
        gunReact = _gunReact;
    }
    int getGunReact()
    {
        return gunReact;
    }
    SpacePoint getShipPoint()
    {
        return shipPoint;
    }
    int getXShipPoint()
    {
        return shipPoint.getX();
    }
    int getYShipPoint()
    {
        return shipPoint.getY();
    }
    int getCredits()
    {
        return accaunt205.getCredits();
    }
    void setFuelLeft(int _fuelLeft)
    {
        fuelLeft = _fuelLeft;
    }
    void setFuelMax(int _fuelMax)
    {
        fuelMax = _fuelMax;
    }
    void setName(string _name)
    {
        name = _name;
    }
    void setShipPoint(SpacePoint _shipPoint)
    {
        shipPoint = _shipPoint;
    }
    void setCredits(int _credits)
    {
        accaunt205.setCredits(_credits);
    }
    void flight(SpacePoint _spacePoint)
    {
        fuelLeft -= shipPoint.distance(_spacePoint);
        if (fuelLeft < 0)
            fuelLeft = 0;
        shipPoint = _spacePoint;
    }
    int shoot()
    {
        gunReact -= 5;
        cd.drawLine(Point(shipPoint.getX(), shipPoint.getY()), Point(shipPoint.getX() + 100 * cos(gunAngle), shipPoint.getY() + 100 * sin(gunAngle)), ConsolColor::yellow());
        
        return 0;
    }
    void drawSpaceShip(COLORREF color)
    {
        cd.drawCircle(Point(shipPoint.getX(), shipPoint.getY()), 20, color, true);
        //cd.drawCircle(Point(shipPoint.getX(), shipPoint.getY()), 20, ConsoleColor::white());
        cd.drawLine(Point(shipPoint.getX() - 20, shipPoint.getY()), Point(shipPoint.getX() + 20, shipPoint.getY()), ConsoleColor::white());
        cd.drawLine(Point(shipPoint.getX(), shipPoint.getY() - 20), Point(shipPoint.getX(), shipPoint.getY() + 20), ConsoleColor::white());
        cd.drawLine(Point(shipPoint.getX() + 14, shipPoint.getY() - 14), Point(shipPoint.getX() - 14, shipPoint.getY() + 14), ConsoleColor::white());
        cd.drawLine(Point(shipPoint.getX() - 14, shipPoint.getY() - 14), Point(shipPoint.getX() + 14, shipPoint.getY() + 14), ConsoleColor::white());

    }
    void printStatus()
    {
        cout << "STATUS: " << name << endl
            << "Cordinates: x=" << shipPoint.getX() << " y=" << shipPoint.getY() << endl
            << "Fuel: " << fuelLeft << " from " << fuelMax << endl
            << "Account: " << accaunt205.getCredits() << " credits" << endl;
    }
};

//class neutSpaceShip : public SpaceShip {};
//{
//private:
//
//public:
//    neutSpaceShip()
//
//};

SpaceShip EGOSOFT("EGOSOFT", 5000);//наш корабль

class SpaceStation {
private:
    string name;
    SpacePoint statPoint;
    Product productSell, productBuy;
public:
    SpaceStation(int _x = 400, int _y = 400, string _name = "rand")
    {
        statPoint.setX(_x);
        statPoint.setY(_y);
        if (_name == "rand")
            name = statNames[rand() % statNames.size()];
        else
            name = _name;
    }
    SpaceStation(Product _productSell, Product _productBuy, int _x = 400, int _y = 400, string _name = "rand")
    {
        statPoint.setX(_x);
        statPoint.setY(_y);
        productSell.setName(_productSell.getName());
        productSell.setCount(_productSell.getCount());
        productSell.setPrice(_productSell.getPrice());
        productBuy.setName(_productBuy.getName());
        productBuy.setCount(_productBuy.getCount());
        productBuy.setPrice(_productBuy.getPrice());
        if (_name == "rand")
            name = statNames[rand() % statNames.size()];
        else
            name = _name;
    }
    void setProductSell(string _name, int _count, int _cena)
    {
        productSell.setName(_name);
        productSell.setCount(_count);
        productSell.setPrice(_cena);
    }
    void setProductBuy(string _name, int _count, int _cena)
    {
        productBuy.setName(_name);
        productBuy.setCount(_count);
        productBuy.setPrice(_cena);
    }
    int getXStatPoint()
    {
        return statPoint.getX();
    }
    int getYStatPoint()
    {
        return statPoint.getY();
    }
    void buyProduct(int _count)
    {
        if (EGOSOFT.getCredits() >= productSell.getPrice() * _count)
        {
            EGOSOFT.setCredits(EGOSOFT.getCredits() - productSell.getPrice() * _count);
            EGOSOFT.cargo.addCargo(Product(productSell.getName(), productSell.getPrice(), _count));
            cout << "You bought " << _count << " " << productSell.getName();
        }
        else
            cout << "not enough money" << endl;
    }
    void sellProduct(int _count)
    {
        int p = -1;
        for (int i = 0; i < EGOSOFT.cargo.products.size(); i++)
            if ((productBuy.getName() == EGOSOFT.cargo.products[i].getName()) && (EGOSOFT.cargo.products[i].getCount() >= _count))
                p = i;

        if ((EGOSOFT.cargo.products[p].getCount() >= _count) && (p != -1))
        {
            EGOSOFT.cargo.delCargo(Product(productBuy.getName(), productBuy.getPrice(), _count));
            EGOSOFT.setCredits(EGOSOFT.getCredits() + productBuy.getPrice() * _count);
            cout << "You sold " << _count << " " << productBuy.getName();
        }
        else
            cout << "not enough cargo" << endl;
    }
    void fillupShip(SpaceShip& ship)
    {
        ship.setCredits(ship.getCredits() - (ship.getFuelMax() - ship.getFuelLeft()) / 100);
        ship.setFuelLeft(ship.getFuelMax());
        cout << "Your ship was filled up with fuel";
    }
    void drowSpaceStation(COLORREF color)
    {
        cd.drawCircle(Point(statPoint.getX(), statPoint.getY()), 50, color, true);
        cd.drawCircle(Point(statPoint.getX(), statPoint.getY()), 40, ConsoleColor::black(), true);

    }
    void print_status()
    {
        cout << "STATUS: " << name << endl
            << "Cordinates: x=" << statPoint.getX() << " y=" << statPoint.getY() << endl
            << "Products: " << productSell.getCount() << " " << productSell.getName() << " for price " << productSell.getPrice() << endl
            << "Resourses: need " << productBuy.getCount() << " " << productBuy.getName() << " for price " << productBuy.getPrice() << endl;
    }
};

class SpaceSector
{
private:
public:
    //великие и могучие вектора...
    //объекты космоса привязанные к сектору
    vector <Point> stars;
    vector <Circle> planets;
    vector <SpaceStation> stations;

    SpaceSector()
    {
        //да будут звёзды
        int pointCount = 1000;
        for (int i = 0; i < pointCount; i++)
            stars.push_back(Point(rand() % cd.getWidth(), rand() % cd.getHeight()));

        //создаём планетЫ
        int random = rand() % 6;
        if (random <= 2)
            planets.push_back(Circle(Point(rand() % cd.getWidth(), rand() % cd.getHeight()), rand() % 450, ConsoleColor::random_color()));
        else if (random <= 4)
        {
            planets.push_back(Circle(Point(rand() % cd.getWidth(), rand() % cd.getHeight()), rand() % 450, ConsoleColor::random_color()));
            planets.push_back(Circle(Point(rand() % cd.getWidth(), rand() % cd.getHeight()), rand() % 450, ConsoleColor::random_color()));
        }
        else if (random == 5)
        {
            planets.push_back(Circle(Point(rand() % cd.getWidth(), rand() % cd.getHeight()), rand() % 450, ConsoleColor::random_color()));
            planets.push_back(Circle(Point(rand() % cd.getWidth(), rand() % cd.getHeight()), rand() % 450, ConsoleColor::random_color()));
            planets.push_back(Circle(Point(rand() % cd.getWidth(), rand() % cd.getHeight()), rand() % 450, ConsoleColor::random_color()));
        }

        //создаём станции
        for (int i = 0; i < (rand() % 5 + 5); i++)
            stations.push_back(SpaceStation(Product(products[rand() % 5], rand() % 500, rand() % 500), Product(products[rand() % 5], rand() % 500, rand() % 500), rand() % cd.getWidth(), rand() % cd.getHeight()));

    }
    void drawSector()
    {
        cd.drawBackground();//рисуем фон

        for (int i = 0; i < stars.size(); i++)//рисуем звёзды
        {
            for (int j = 0; j < planets.size(); j++)
            {
                if (!planets[j].isInside(stars[i]))
                {
                    cd.drawPoint(stars[i], ConsoleColor::white());
                }
            }
        }

        for (int i = 0; i < planets.size(); i++)//рисуем планетЫ
        {
            cd.drawCircle(Point(planets[i].getCenter().getX(), planets[i].getCenter().getY()), planets[i].getRadius(), planets[i].getColor(), true);
        }


        for (int i = 0; i < stations.size(); i++)//рисуем станции
        {
            stations[i].drowSpaceStation(ConsoleColor::blue());
        }

    }
};

int mapI = 0, mapJ = 0;

class SpaceGates
{
private:
    SpacePoint gatesPoint;
public:
    SpaceGates(SpacePoint _gatesPoint)
    {
        gatesPoint = _gatesPoint;
    }
    int getX()
    {
        return gatesPoint.getX();
    }
    int getY()
    {
        return gatesPoint.getY();
    }
    void teleport(char direction)
    {
        if ((direction == 'n') && (mapI > 0))
            mapI--;
        else if ((direction == 's') && (mapI < 9))
            mapI++;
        else if ((direction == 'w') && (mapJ > 0))
            mapJ--;
        else if ((direction == 'e') && (mapJ < 9))
            mapJ++;
    }
    void drowSpaceGates()
    {
        cd.drawCircle(Point(gatesPoint.getX(), gatesPoint.getY()), 30, ConsoleColor::portal(), true);
    }
};

int main()
{
	srand(time(0));

    cd.setWidth(1920);//размер экрана
    cd.setHeight(1080);

    string command;
    int index;//банальные переменные для банальных решений

    vector <vector <SpaceSector>> map;//создаём карту 10 x 10
    for (int i = 0; i < 10; i++)
    {
        vector <SpaceSector> temp;
        for (int j = 0; j < 10; j++)
        {
            SpaceSector sector;
            temp.push_back(sector);
        }
        map.push_back(temp);
    }

    EGOSOFT.setShipPoint(SpacePoint(100, 100));//спавним наш корабль

    SpaceGates N(SpacePoint(960, 30));//врата из X
    SpaceGates S(SpacePoint(960, 1050));
    SpaceGates W(SpacePoint(30, 540));
    SpaceGates E(SpacePoint(1890, 540));

    bool inStat = 0;

    while (cd.isWork())/*------------прорисовка-----------*/
    {
        map[mapI][mapJ].drawSector();//рисуем текущий сектор
        EGOSOFT.drawSpaceShip(ConsoleColor::green());//рисуем наш корабль
        N.drowSpaceGates();
        S.drowSpaceGates();//рисуем порталы
        W.drowSpaceGates();
        E.drowSpaceGates();

        /*--------------------------------------------------------------*/
        //события

        for (int i = 0; i < map[mapI][mapJ].stations.size(); i++)//посадка на станцию
        {
            GetKEY();
            if ((abs(EGOSOFT.getXShipPoint() - map[mapI][mapJ].stations[i].getXStatPoint()) <= 30) && (abs(EGOSOFT.getYShipPoint() - map[mapI][mapJ].stations[i].getYStatPoint()) <= 30) && KEY[13])
            {
                inStat = 1;
                system("CLS");
                while (inStat)
                {
                    cout << "введи команду" << endl;
                    cin >> command;
                    if (command == "fuel")
                        map[mapI][mapJ].stations[i].fillupShip(EGOSOFT);
                    else if (command == "buy")
                    {
                        cin >> index;
                        map[mapI][mapJ].stations[i].buyProduct(index);
                    }
                    else if (command == "sell")
                    {
                        cin >> index;
                        map[mapI][mapJ].stations[i].sellProduct(index);
                    }
                    else if (command == "status")
                    {
                        cin >> command;
                        if (command == "ship")
                            EGOSOFT.printStatus();
                        if (command == "station")
                            map[mapI][mapJ].stations[i].print_status();
                    }
                    else if (command == "exit")
                    {
                        inStat = 0;
                    }
                }
            }
            else inStat = 0;
        }

        if ((abs(EGOSOFT.getXShipPoint() - N.getX()) <= 30) && (abs(EGOSOFT.getYShipPoint() - N.getY()) <= 30))//влетание во врата
        {
            N.teleport('n');
            EGOSOFT.flight(SpacePoint(960, 1010));
        }
        else if ((abs(EGOSOFT.getXShipPoint() - S.getX()) <= 30) && (abs(EGOSOFT.getYShipPoint() - S.getY()) <= 30))
        {
            S.teleport('s');
            EGOSOFT.flight(SpacePoint(960, 70));
        }
        else if ((abs(EGOSOFT.getXShipPoint() - W.getX()) <= 30) && (abs(EGOSOFT.getYShipPoint() - W.getY()) <= 30))
        {
            W.teleport('w');
            EGOSOFT.flight(SpacePoint(1850, 540));
        }
        else if ((abs(EGOSOFT.getXShipPoint() - E.getX()) <= 30) && (abs(EGOSOFT.getYShipPoint() - E.getY()) <= 30))
        {
            E.teleport('e');
            EGOSOFT.flight(SpacePoint(70, 540));
        }

        if (EGOSOFT.getGunReact() <= 100)
            EGOSOFT.setGunReact(EGOSOFT.getGunReact() + 1);

        GetKEY();//нажатие клавиш
        if (KEY[87] && !inStat) EGOSOFT.flight(SpacePoint(EGOSOFT.getXShipPoint(), EGOSOFT.getYShipPoint() - 20)); //W
        if (KEY[65] && !inStat) EGOSOFT.flight(SpacePoint(EGOSOFT.getXShipPoint() - 20, EGOSOFT.getYShipPoint())); //A
        if (KEY[83] && !inStat) EGOSOFT.flight(SpacePoint(EGOSOFT.getXShipPoint(), EGOSOFT.getYShipPoint() + 20)); //S
        if (KEY[68] && !inStat) EGOSOFT.flight(SpacePoint(EGOSOFT.getXShipPoint() + 20, EGOSOFT.getYShipPoint())); //D
        if (KEY[32] && !inStat) EGOSOFT.shoot(); //SPACE
        if (KEY[27] && !inStat) system("exit"); //ESC
        cd.processLoop(100);
    }
    system("pause");
    return 0;
}
