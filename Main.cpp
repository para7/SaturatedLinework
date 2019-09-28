
#include <Siv3D.hpp>

//角度の正規化 0~2π
constexpr double AngleNormalize(double angle)
{
    while(angle < 0)
    {
        angle += Math::Pi * 2;
    }
    
    return fmod(angle, Math::Pi * 2);
}

Ellipse::position_type GetPointOnLine(const Ellipse& ellipse, double angle)
{
    auto x = cos(angle);
    auto y = -sin(angle);
    
    return Ellipse::position_type(x * ellipse.a, y * ellipse.b).moveBy(ellipse.center);
}

template <class T>
static Vec2 GetPointOnLine(const Rectangle<T>& rect, double angle)
{
    //line begin, line end
    Vec2 lbegin, lend;
    constexpr auto pi4 = Math::Pi / 4;
    
    angle = AngleNormalize(angle);
    
    //辺を選択する
    //topとbottomが入れ替わっているのは時計回りに配置し直したいため
    //ついでに角度を0~1/2πに正規化
    if(angle < pi4 || 7 * pi4 < angle)
    {
        const auto line = rect.right();
        lbegin = line.end;
        lend = line.begin;
        angle += pi4;
        angle = AngleNormalize(angle);
    }
    else if(angle < 3 * pi4)
    {
        const auto line = rect.top();
        lbegin = line.end;
        lend = line.begin;
        angle -= pi4;
    }
    else if(angle < 5 * pi4)
    {
        const auto line = rect.left();
        lbegin = line.end;
        lend = line.begin;
        angle -= pi4 * 3;
    }
    else
    {
        const auto line = rect.bottom();
        lbegin = line.end;
        lend = line.begin;
        angle -= pi4 * 5;
    }
    //角度をさらに0~1に正規化
    const auto v = (angle * 2) / Math::Pi;
    
    //線形補間で座標を出す
    return lbegin.lerp(lend, v);
}


// 集中線クラス
template <class InnerShape, class OuterShape>
class ConcentratedEffect
{
public:
    
    //内側の図形
    InnerShape innershape;
    //外側の図形
    OuterShape outershape;
    
    //線の数
    int linenum = 60;
    
    //線の太さ 中心からの角度
    double thicknessangle = 0.01;
    
    //太さのランダム幅
    double thickrandomness = 0.003;
    
    //出現位置のランダム幅
    double posrandomness = 0;
    
private:
    
    Array<Triangle> m_triangles;
    
public:
    
    ConcentratedEffect(const InnerShape& _innershape, const OuterShape& _outershape)
    : innershape(_innershape)
    , outershape(_outershape)
    {
        Generate();
    }
//    
//    ConcentratedEffect(const InnerShape& _innershape, const OuterShape& _outershape)
//    : innershape(_innershape)
//    , outershape(_outershape)
//    {
//        (Vec2(Scene::Size()) / 2, Vec2(Scene::Size()) * sqrt(2.0));
//    }
    
    
    //outershapeを指定しない場合は画面をカバーするように自動設定する
    //    ConcentratedEffect(const InnerShape& _innershape)
    //        : innershape(_innershape)
    //        , outershape(Rect(0, 0, Scene::Size()))
    //    {
    //        Generate();
    //    }
    
    //集中線を生成する
    void Generate()
    {
        m_triangles.clear();
        m_triangles.reserve(linenum);
        
        for(int i : step(linenum))
        {
            const double angle = Random(2 * Math::Pi);
            
            auto inner = GetPointOnLine(innershape, angle);
            
            auto outerleft = GetPointOnLine(outershape, angle + thicknessangle);
            
            auto outerright = GetPointOnLine(outershape, angle - thicknessangle);
            
            m_triangles.emplace_back(inner, outerleft, outerright);
        }
    }
    
    void draw(const Color& color) const
    {
        for(const auto& triangle : m_triangles)
        {
            triangle.draw(color);
        }
    }
};

void Main()
{
    Window::Resize(1280, 720);
    
    Ellipse el(400, 350, 100, 50);
    Ellipse el2(400, 350, 100*3, 50*3);
    //Ellipse windowel(Vec2(Scene::Size()) / 2, Vec2(Scene::Size()) * sqrt(2.0));
    
    ConcentratedEffect effect(el, el2);
    
    Scene::SetBackground(HSV(0, 0, 0.9));
    
    Font font(50, Typeface::Default, FontStyle::BoldItalic);
    
    double num = 0.3;
    
    while(System::Update())
    {
        
        font(U"集中線").drawAt(el.center, Palette::Black);
        
        effect.draw(Palette::Black);
        
        SimpleGUI::Slider(U"{:.0f}"_fmt(num), num, 0.0, 300.0, Vec2(100, 80), 60, 150);
        
        effect.linenum = static_cast<int>(num);
        
        if(SimpleGUI::Button(U"Generate", Vec2(100, 40)))
        {
            effect.Generate();
        }
    }
}
