
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
    int linenum = 70;
    
    //線の太さ 中心からの角度
    double thickness = 7;
    
    //太さのランダム幅
    double thickrandomness = 16;
    
    //出現位置のランダム幅
    double posrandomness = 40;
    
private:
    
    Array<Triangle> m_triangles;
    
public: //constructor
    
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
    
public: //func
    
    //集中線を生成する
    void Generate()
    {
        //内部の図形の方が外部の図形より大きい場合はエラー
        //        if(!outershape.contains(innershape))
        //        {
        //            Print(U"error");
        //            return;
        //            //エラー処理
        //        }
        
        m_triangles.clear();
        m_triangles.reserve(linenum);

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
            const auto is = innershape.stretched(Random(posrandomness));
            //InnerShapeの中心から線を始めているので、intersectsAtの存在判定は不要
            const Vec2 inner = is.intersectsAt(line).value().front();
            
            //外側の中心(基準)となる座標を計算する
            //こっちは存在しない可能性があるが、 outershape が innershape を内包しているかどうかで判定すべきである
            //ただしEllipse.contains()はないのでどうするか
            const auto outer = outershape.intersectsAt(line).value().front();
            
            //こっちも極座標にしようとしたら座標計算ミスしまくったのでとりあえずこのままで
            
            //90度回す
            const auto rotated = angle + (Math::Pi / 2);
            
            //ずらす量は太さの半分
            const double r = ( (thickness + Random(thickrandomness)) / 2);
            
            //単位ベクトルを生成
            const Vec2 v(cos(rotated), sin(rotated));
            
            //実際のずらす量を計算
            const Vec2 outeroffset = v * r;
            
            //左右にずらした座標を作る
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
    
    void GenerateDraw(const Color& color) const
    {
        
    }
};

void Main()
{
    Window::Resize(1280, 720);
    
    Ellipse el(400, 400, 300, 100);
    //Ellipse el2 = el.stretched(800);
    
    ConcentratedEffect effect(el);
    
    Scene::SetBackground(HSV(0, 0, 0.93));
    
    Font font(90, Typeface::Default, FontStyle::BoldItalic);
    
    double num = effect.linenum;
    double thick = effect.thickness;
    double posrandom = effect.posrandomness;
    double thickrandom = effect.thickrandomness;
    
    while(System::Update())
    {
        font(U"集中線").drawAt(el.center, Palette::Black);
        
        effect.draw(Palette::Black);
        
        const int label = 130;
        const int slider = 250;
        SimpleGUI::Slider(U"num{:.0f}"_fmt(num), num, 0.0, 300.0, Vec2(100, 80), label, slider);
        SimpleGUI::Slider(U"thick{:.0f}"_fmt(thick), thick, 0.0, 80.0, Vec2(100, 80 + 40 * 1), label, slider);
        SimpleGUI::Slider(U"posRand{:.0f}"_fmt(posrandom), posrandom, 0.0, 400.0, Vec2(100, 80 + 40 * 2), label, slider);
        SimpleGUI::Slider(U"thickRand{:.0f}"_fmt(thickrandom), thickrandom, 0.0, 50.0, Vec2(100, 80 + 40 * 3), label, slider);
        
        effect.linenum = static_cast<int>(num);
        effect.thickness = static_cast<int>(thick);
        effect.thickrandomness = static_cast<int>(thickrandom);
        effect.posrandomness = static_cast<int>(posrandom);
        
        if(SimpleGUI::Button(U"Generate", Vec2(100, 40)))
        {
            effect.Generate();
        }
        if(SimpleGUI::Button(U"RandomSet", Vec2(260, 40)))
        {
            effect.Generate();
        }
    }
}
