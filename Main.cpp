
#include <Siv3D.hpp>

template <class InnerShape, class OuterShape>
struct ConcentratedEffect
{
    InnerShape innershape;
    OuterShape outershape;
    
    int linenum;
    double thickness = 10;
    
private:
    
    Array<Triangle> m_triangles;
    
public:
    
    ConcentratedEffect(const InnerShape& _innershape, const OuterShape& _outershape)
        : innershape(_innershape)
        , outershape(_outershape)
    {
        linenum = 60;
        
        Generate();
    }
   
    
    //outershapeを指定しない場合は画面をカバーするように自動設定する
//    ConcentratedEffect(const InnerShape& _innershape)
//        : innershape(_innershape)
//        , outershape(Rect(0, 0, Scene::Size()))
//    {
//        Generate();
//    }
    
    static Ellipse::position_type GetPointOnLine(const Ellipse& ellipse, double angle)
    {
        auto x = cos(angle);
        auto y = -sin(angle);
        
        return Ellipse::position_type(x * ellipse.a, y * ellipse.b).moveBy(ellipse.center);
    }
    
    template<class _Rectangle>
    static Vec2 GetPointOnLine(const _Rectangle& rectangle, double angle)
    {
        
    }
    
    //集中線を生成する
    void Generate()
    {
        m_triangles.clear();
        m_triangles.reserve(linenum);
        
        for(int i : step(linenum))
        {
            double angle = Random(2 * Math::Pi);
            
            auto inner = GetPointOnLine(innershape, angle);
            auto outer = GetPointOnLine(outershape, angle);

            auto shifted_angle = angle - Math::Pi/2;
            auto outerleft = outer.movedBy(cos(shifted_angle) * thickness, sin(shifted_angle) * thickness);
            
            shifted_angle = angle + Math::Pi/2;
            auto outerright = outer.movedBy(cos(shifted_angle) * thickness, sin(shifted_angle) * thickness);
            
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
    
    Ellipse el(400, 400, 200, 100);
    Ellipse windowel(Vec2(Scene::Size()) / 2, Vec2(Scene::Size()) * sqrt(2.0));
    
    ConcentratedEffect effect(el, windowel);
    
    Scene::SetBackground(HSV(0, 0, 0.9));
    
    Font font(130, Typeface::Default, FontStyle::BoldItalic);

    
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
