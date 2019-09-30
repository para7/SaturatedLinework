
#include <Siv3D.hpp>
#include <HamFramework.hpp>

//角度の正規化 0~2π
double AngleNormalize(double angle)
{
    while(angle < 0)
    {
        angle += Math::Pi * 2;
    }
    
    return fmod(angle, Math::Pi * 2);
}

//Ellipseの線上の点を求める
Ellipse::position_type GetPointOnLine(const Ellipse& ellipse, double angle)
{
    auto x = cos(angle);
    auto y = -sin(angle);
    
    return Ellipse::position_type(x * ellipse.a, y * ellipse.b).moveBy(ellipse.center);
}

//Rectの線上の点を求める
template <class T>
static Vec2 GetPointOnLine(const Rectangle<T>& rect, double angle)
{
    constexpr auto pi4 = Math::Pi / 4;
    
    angle = AngleNormalize(angle);
    
    Line line;
    
    //辺を選択する
    //ついでに角度を0~1/2πに正規化する
    if(angle < pi4 || 7 * pi4 < angle)
    {
        line = rect.right();
        angle += pi4;
        angle = AngleNormalize(angle);
    }
    else if(angle < 3 * pi4)
    {
        line = rect.top();
        angle -= pi4;
    }
    else if(angle < 5 * pi4)
    {
        line = rect.left();
        angle -= pi4 * 3;
    }
    else
    {
        line = rect.bottom();
        angle -= pi4 * 5;
    }
    
    //角度をさらに0~1に正規化
    const auto v = (angle * 2) / Math::Pi;
    
    //線形補間で座標を出す
    return line.end.lerp(line.begin, v);
}


// 集中線クラス
template <class InnerShape, class OuterShape = Rect>
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
    double thickness = 6;
    
    //太さのランダム幅
    double thickrandomness = 0;
    
    //出現位置のランダム幅
//    double posrandomness = 100;
    
private:
    
    Array<Triangle> m_triangles;
    
public:
    
    //集中線を生成する
    void Generate()
    {
        m_triangles.clear();
        m_triangles.reserve(linenum);
     
        if(!outershape.contains(innershape))
        {
            Print(U"error");
            return;
            //エラー処理
        }
        
        for(int i : step(linenum))
        {
            //角度を作成
            const double angle = Random(2 * Math::Pi);
            
            //内側の座標
            //まずはランダム性を持たせる
            //const auto is = innershape.stretched(Random(posrandomness));
            //const auto inner = GetPointOnLine(is, angle);
            
            const auto inner = GetPointOnLine(innershape, angle);
            
            //外側の座標を計算する
            const auto outer = GetPointOnLine(outershape, angle);
            
            //90度回す
            const auto rotated = angle + (Math::Pi / 2);
            
            //太さの分ずらすベクトルを作成
            const auto r = ( (thickness + Random(thickrandomness)) / 2);
            const Vec2 v(cos(rotated), -sin(rotated));
            const auto outeroffset = v * r;
            
            //ずらした座標を作る
            const auto outerleft = outer + outeroffset;
            const auto outerright = outer - outeroffset;
            
            //Triangleの座標として登録
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
    
    ConcentratedEffect(const InnerShape& _innershape, const OuterShape& _outershape)
    : innershape(_innershape)
    , outershape(_outershape)
    {
        Generate();
    }
    
    //outershapeを指定しない場合は画面をカバーするように自動設定する
    ConcentratedEffect(const InnerShape& _innershape)
    : innershape(_innershape)
    , outershape(Rect(0, 0, Scene::Size()).stretched(20, 20))
    {
        Generate();
    }
};

void Main()
{
    Window::Resize(1280, 720);
    
    Ellipse el(200, 500, 200, 100);
//    Ellipse el2(400, 350, 100*3, 50*3);
    
    ConcentratedEffect effect(el);
    
    Scene::SetBackground(HSV(0, 0, 0.9));
    
    Font font(90, Typeface::Default, FontStyle::BoldItalic);
    
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
