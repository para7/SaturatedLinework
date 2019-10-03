
#include <Siv3D.hpp>
#include <HamFramework.hpp>

//角度の正規化 0~2π
double AngleNormalize(double angle)
{
    const auto ret = fmod(angle, Math::Pi * 2);
    return ret < 0 ? ret : ret + Math::Pi * 2;
}

// 集中線クラス
template <class InnerShape = Ellipse, class OuterShape = Rect>
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
    
    ConcentratedEffect(){};
    
    //outershapeを指定しない場合は画面をカバーするように自動設定する
    //端の方が見えてしまうので画面より少し大きく
    ConcentratedEffect(const InnerShape& _innershape)
    : innershape(_innershape)
    , outershape(Rect(0, 0, Scene::Size()).stretched(30, 30))
    {
        Generate();
    }
    
    //外側の図形を指定するタイプ
    //実装上、端の方が汚くなってしまっているので使う場面はないかも
    //前に作っていた角度で太さを指定する方法だと端を図形ぴったりで生成できたのですが
    //とても指定しづらいですし、大問題が起きたので没です
    ConcentratedEffect(const InnerShape& _innershape, const OuterShape& _outershape)
    : innershape(_innershape)
    , outershape(_outershape)
    {
        Generate();
    }
    
private:
    
    //図形によってcenter()とcenterで異なるのでこれで吸収する
    template < typename T >
    static auto GetCenter(const T& x) -> decltype(x.center())
    {
        return x.center();
    }
    
    template < typename T >
    static auto GetCenter(const T& x) -> decltype(x.center)
    {
        return x.center;
    }
    
    
public: //func
    
    
    //追加機能案：お気に入りの集中線をいつでも作れるよう、
    //乱数の初期値を受け取るようにしたりしてもいいかもしれない。
    //集中線を生成する
    void Generate()
    {
        //内部の図形が外部の図形に包まれてない場合はエラー
        //        if(!outershape.contains(innershape))
        //        {
        //            Print(U"error");
        //            return
        //            //エラー処理
        //        }
        // Ellipseのcontains等がないので動かない
        // 将来的にコメントアウトを外せるようになるかもしれませんね
        
        //最初に配列をリセットする
        m_triangles.clear();
        m_triangles.reserve(linenum);
        
        //集中線の位置は実際に線を置いてみて座標を生成します。
        //交差座標取得用
        OffsetCircular3 cir;
        //半径を長く取る
        //画面より大きければよほどのことがない限り平気なはず
        //もうちょっと意味のある値にしたいところだが、オーバーフローも警戒しないといけない。
        cir.r = 1000000;
        
        for(int i : step(linenum))
        {
            //極座標を初期化
            double angle = Random(2 * Math::Pi);
            cir.theta = angle;
            cir.setCenter(GetCenter(innershape));
            
            //中心線を作る
            Line line(cir.center, cir);
            
            //内側の座標
            //まずは位置にランダム性を持たせる
            const auto is = innershape.stretched(Random(posrandomness));
            //InnerShapeの中心から線を始めているので絶対交差すると言いたかったが
            //図形があまりにも大きいと交差しないので例外を吐かないよう判定をとっておく
            
            const auto innerintersects = is.intersectsAt(line);
            
            if(!innerintersects)
            {
                continue;
            }
            
            //内側は太さは必要ないので中心線の座標をそのまま使用
            const Vec2 inner = innerintersects.value().front();
            
            //外側の中心(基準)となる座標を計算する
            const auto outerintersects = outershape.intersectsAt(line);
            
            //交差してなかったらエラー
            if(!outerintersects)
            {
                continue;
                //エラーを吸って無理やり続行する仕様になっており
                //なぜ集中線が生成されなかったのか？がわかりづらいので
                //例外等でお知らせしてもよいと思う
                //そのあたりの仕様はOpenSiv3D内で規則のようなものがあると思ったので
                //手をつけていません。
            }
            
            //外側の図形がとんでもない位置にあった場合は
            //2箇所以上交差する可能性があるが、その処理は現段階では作っていない。
            const auto outer = outerintersects.value().front();
            
            //innerから引いた直線から垂直に、outerの座標から太さの半分ずらした点を2つ生成する処理
            //90度回す
            const auto rotated = angle + (Math::Pi / 2);
            
            //ずらす量は太さの半分
            const double r = ( (thickness + Random(thickrandomness)) / 2);
            
            //単位ベクトルを生成
            const Vec2 v(cos(rotated), sin(rotated));
            
            //実際のずらす量を計算
            const Vec2 outeroffset = v * r;
            
            //このあたりも極座標にしようと書き換えたところ、座標がぶっ飛んでいったので
            //動作としては問題ありませんし他の仕様等を完成させるべきと考え
            //以前書いたままにしてあります。
            
            //左右にずらした座標を作る
            const Vec2 outerleft = outer + outeroffset;
            const Vec2 outerright = outer - outeroffset;
            
            //Triangleの座標として登録
            m_triangles.emplace_back(inner, outerleft, outerright);
        }
    }
    
    void draw(const Color& color = Palette::Black) const
    {
        for(const auto& triangle : m_triangles)
        {
            triangle.draw(color);
        }
    }
    
    //集中線を毎フレーム生成すると見た目が派手になるので
    //GenerateしつつDrawする機能があってもいいかと思ったのですが
    //本当に必要なのかちょっと考えています。
    //現状動かせませんが、Generate()からGenarate(Array<Triangle>& array)constに中身を移動し
    //Generate()はGenerate()constを呼び出すよう変更すれば可能です。
    //機能的に蛇足となってしまうかもしれないのでとりあえず保留。
    //    void GenerateDraw(const Color& color) const
    //    {
    //
    //    }
};

struct
{
    double value;
    double max;
    double min = 0;
}num,thick,posrandom,thickrandom;

void Main()
{
    Window::Resize(1280, 720);
    
    Ellipse el(500, 400, 200, 100);
    
    ConcentratedEffect effect(el);
    
    //実装変えたのでEllipseじゃなくてもintersectsAtが使えれば動きます。
    //Rect r(500, 400, 200, 100);
    //ConcentratedEffect effect(r);
    
    
    //外側を指定することも可能です。
    //こちらもどんな図形でもできるはず。
    //Ellipse el2 = el.stretched(400);
    //ConcentratedEffect effect(el, el2);
    
    
    Scene::SetBackground(HSV(0, 0, 0.93));
    
    Font font(90, Typeface::Default, FontStyle::BoldItalic);
    
    num.value = effect.linenum;
    thick.value = effect.thickness;
    posrandom.value = effect.posrandomness;
    thickrandom.value = effect.thickrandomness;

    num.max = 300;
    thick.max = 80;
    posrandom.max = 400;
    thickrandom.max = 50;
    
    while(System::Update())
    {
        font(U"集中線").drawAt(el.center, Palette::Black);
        
        //Slider GUIのサイズ
        const int label = 130;
        const int slider = 250;
        //GUIのベース座標
        const int x = 100;
        const int y = 80;
        
        effect.linenum = static_cast<int>(num.value);
        effect.thickness = static_cast<int>(thick.value);
        effect.thickrandomness = static_cast<int>(thickrandom.value);
        effect.posrandomness = static_cast<int>(posrandom.value);
        
        
        effect.draw(Palette::Black);
        SimpleGUI::Slider(U"num{:.0f}"_fmt(num.value), num.value, num.min, num.max, Vec2(x, 80), label, slider);
        SimpleGUI::Slider(U"thick{:.0f}"_fmt(thick.value), thick.value, thick.min, thick.max, Vec2(x, 80 + 40 * 1), label, slider);
        SimpleGUI::Slider(U"posRand{:.0f}"_fmt(posrandom.value), posrandom.value, posrandom.min, posrandom.max, Vec2(x, 80 + 40 * 2), label, slider);
        SimpleGUI::Slider(U"thickRand{:.0f}"_fmt(thickrandom.value), thickrandom.value, thickrandom.min, thickrandom.max, Vec2(x, 80 + 40 * 3), label, slider);
        
        if(SimpleGUI::Button(U"Generate", Vec2(x, 40)))
        {
            effect.Generate();
        }
        if(SimpleGUI::Button(U"RandomSet", Vec2(x + 140, 40)))
        {
            num.value = Random(num.min, num.max);
            thick.value = Random(thick.min, thick.max);
            posrandom.value = Random(posrandom.min, posrandom.max);
            thickrandom.value = Random(thickrandom.min, thickrandom.max);
            effect.Generate();
        }
    }
}
