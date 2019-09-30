
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
    double thickness = 10;
    
    //太さのランダム幅
    double thickrandomness = 7;
    
    //出現位置のランダム幅
    double posrandomness = 50;
    
private:
    
    Array<Triangle> m_triangles;
    
public:
    
    //集中線を生成する
    void Generate()
    {
        m_triangles.clear();
        m_triangles.reserve(linenum);

        //内部の図形の方が外部の図形より大きい場合はエラー
//        if(!outershape.contains(innershape))
//        {
//            Print(U"error");
//            return;
//            //エラー処理
//        }
        
        //交差座標取得用
        OffsetCircular3 cir;
        //半径を長く取る
        cir.r = 1000000;
        
        for(int i : step(linenum))
        {
            //極座標を初期化
            double angle = Random(2 * Math::Pi);
            cir.theta = angle;
            
            cir.setCenter(innershape.center);
            Line line(cir.center, cir);
            
            //内側の座標
            //まずはランダム性を持たせる
            //const auto is = innershape.stretched(Random(posrandomness));
            //const auto inner = GetPointOnLine(is, angle);
            
            //InnerShapeの中心から線を始めているので、intersectsAtの存在判定は不要
            const Vec2 inner = innershape.intersectsAt(line).value().front();
            
            //外側の座標を計算する
            //こっちは存在判定が必要だが、outershapeがinnershapeを内包しているかどうかで判定すべきである
            //ただしEllipse.contains()はないのでどうするか
            const auto outer = outershape.intersectsAt(line).value().front();
            
            //90度回す
            const auto rotated = angle + (Math::Pi / 2);
            
            //太さの分ずらすベクトルを作成
            const double r = ( (thickness + Random(thickrandomness)) / 2);
            const Vec2 v(cos(rotated), sin(rotated));
            const Vec2 outeroffset = v * r;
            
            //ずらした座標を作る
            const Vec2 outerleft = outer + outeroffset;
            const Vec2 outerright = outer - outeroffset;
            
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
    
    Ellipse el(200, 500, 300, 100);
    Ellipse el2 = el.stretched(800);
    
    ConcentratedEffect effect(el, el2);
    
    Scene::SetBackground(HSV(0, 0, 0.93));
    
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
