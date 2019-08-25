
#include <Siv3D.hpp>

template <class InnerShape, class OuterShape>
struct ConcentratedEffect
{
    InnerShape innershape;
    OuterShape outershape;
    
    int linenum;
    double thickness = 10;
    
private:
    Array<double> angles;
    
public:
    
    ConcentratedEffect(const InnerShape& _innershape, const OuterShape& _outershape)
        : innershape(_innershape)
        , outershape(_outershape)
    {
        linenum = 60;
        angles.resize(linenum);
        
        Generate();
    }
    
    ConcentratedEffect(const InnerShape& _innershape)
        : innershape(_innershape)
        , outershape(Rect(0, 0, Scene::Size()))
    {
        Generate();
    }
    
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
    
    //集中線を設定する
    void Generate()
    {
        angles.resize(linenum);
        
        for(auto& angle : angles)
        {
            angle = Random(Math::Pi * 2);
        }
    }
    
    void draw(const Color& color) const
    {
        //leftとrightは中心から見た時に左か右か
        Vec2 inner,outer,outerleft,outerright;
        double shifted_angle;
        
        for(auto angle : angles)
        {
            //中心側の点
            inner = GetPointOnLine(innershape, angle);
            outer = GetPointOnLine(outershape, angle);
            
            //innerとouterの線分からThicknessの半分ずらした外側の点2つ
            shifted_angle = angle - Math::Pi/2;
            outerleft = outer.movedBy(cos(shifted_angle) * thickness, sin(shifted_angle) * thickness);
            
            shifted_angle = angle + Math::Pi/2;
            outerright = outer.movedBy(cos(shifted_angle) * thickness, sin(shifted_angle) * thickness);
            
            Triangle(inner, outerleft, outerright).draw(color);
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
