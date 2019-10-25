
//---<Center.cpp>---

#include <HamFramework.hpp>
#include <Siv3D.hpp>

namespace s3d
{
    // v0.4.2 で実装
    Vec2 Center(const Float2& pos)
    {
        return pos;
    }

    Vec2 Center(const Vec2& pos)
    {
        return pos;
    }

    Vec2 Center(const Line& line)
    {
        return line.center();
    }

    Vec2 Center(const Circle& circle)
    {
        return circle.center;
    }

    Vec2 Center(const Ellipse& ellipse)
    {
        return ellipse.center;
    }

    Vec2 Center(const Rect& rect)
    {
        return rect.center();
    }

    Vec2 Center(const RectF& rect)
    {
        return rect.center();
    }

    Vec2 Center(const Triangle& triangle)
    {
        return triangle.centroid();
    }

    // Vec2 Center(const Quad& quad)
    //{
    //    return quad.centroid();
    //}

    Vec2 Center(const RoundRect& roundRect)
    {
        return roundRect.center();
    }

    Vec2 Center(const Polygon& polygon)
    {
        return polygon.centroid();
    }
}

//---</Center.cpp>---

#include <HamFramework.hpp>
#include <Siv3D.hpp>

namespace s3d
{
    template <class InnerShape, class OuterShape>
    bool isContain(InnerShape inner, OuterShape outer)
    {
        // [Siv3D ToDo]
        return true;
    }

    template <class InnerShape = Ellipse, class OuterShape = Rect, class URNG = DefaultRNGType>
    class SaturatedLinework
    {
    private:
        //内側の図形
        InnerShape m_innerShape;
        //外側の図形
        OuterShape m_outerShape;

        //線の数
        size_t m_lineNum = 70;

        //線の太さの最小幅
        double m_minThickness = 7;

        //線の太さの最大幅
        double m_maxThickness = 10;

        //出現位置のランダム幅
        double m_posRandomness = 40;

        uint64 m_seed;

        //集中線の中身
        mutable Array<Triangle> m_triangles;

        //乱数エンジン
        mutable URNG m_rng;

        mutable bool m_isDirty = true;

    private:
        // innerShape　が outerShape に含まれているか簡単なチェック
        template <class _InnerShape, class _OuterShape>
        static bool IsValid(const _InnerShape& innerShape, const _OuterShape& outerShape)
        {
            // [Siv3D ToDo] より厳密に
            return outerShape.contains(Center(innerShape));
        }

    public:  // getter setter
        SaturatedLinework& setInnerShape(const InnerShape& innerShape)
        {
            m_innerShape = innerShape;
            m_isDirty = true;
            return *this;
        }

        const InnerShape& getInnerShape() const noexcept
        {
            return m_innerShape;
        }

        SaturatedLinework& setOuterShape(const OuterShape& outershape)
        {
            m_outerShape = outershape;
            m_isDirty = true;
            return *this;
        }

        const OuterShape& getOuterShape() const noexcept
        {
            return m_outerShape;
        }

        SaturatedLinework& setLineNum(size_t linenum)
        {
            m_lineNum = linenum;
            m_isDirty = true;
            return *this;
        }

        size_t getLineNum() const noexcept
        {
            return m_lineNum;
        }

        SaturatedLinework& setMinThickness(double minthickness)
        {
            m_minThickness = minthickness;
            m_maxThickness = Max(m_minThickness, m_maxThickness);
            m_isDirty = true;
            return *this;
        }

        double getMinThickness() const noexcept
        {
            return m_minThickness;
        }

        SaturatedLinework& setMaxThickness(double maxthickness)
        {
            m_maxThickness = maxthickness;
            m_minThickness = Min(m_minThickness, m_maxThickness);
            m_isDirty = true;
            return *this;
        }

        double getMaxThickness() const noexcept
        {
            return m_maxThickness;
        }

        SaturatedLinework& setThickness(double minthickness, double maxthickness)
        {
            m_minThickness = Min(m_minThickness, m_maxThickness);
            m_maxThickness = Max(m_minThickness, m_maxThickness);
            m_isDirty = true;
            return *this;
        }

        SaturatedLinework& setPosRandomness(double posrandomness)
        {
            m_posRandomness = posrandomness;
            m_isDirty = true;
            return *this;
        }

        double getPosRandomness() const noexcept
        {
            return m_posRandomness;
        }

        SaturatedLinework& setSeed(uint64 seed)
        {
            m_rng.seed(seed);
            m_isDirty = true;
            return *this;
        }

        uint64 getSeed() const noexcept
        {
            return m_seed;
        }

    public:  // constructor
        SaturatedLinework()
            : m_innerShape(Ellipse(Scene::CenterF(), 200, 100))
            , m_outerShape(Scene::Rect().stretched(30, 30))
        {
            m_seed = Random(std::numeric_limits<uint64>::max());
            setSeed(m_seed);
        }

        // outershapeを指定しない場合は画面をカバーするように自動設定する
        //端の方が見えてしまうので画面より少し大きく
        explicit SaturatedLinework(const InnerShape& innerShape)
            : m_innerShape(innerShape)
            , m_outerShape(Scene::Rect().stretched(30, 30))
        {
            m_seed = Random(std::numeric_limits<uint64>::max());
            setSeed(m_seed);
        }

        //外側の図形を指定する
        SaturatedLinework(const InnerShape& innerShape, const OuterShape& outerShape)
            : m_innerShape(innerShape)
            , m_outerShape(outerShape)
        {
            m_seed = Random(std::numeric_limits<uint64>::max());
            setSeed(m_seed);
        }

    public:  // func
        //集中線を生成する
        void Generate() const
        {
            // 内部の図形が外部の図形に包まれてない場合はエラー
            if (!IsValid(m_innerShape, m_outerShape))
            {
                return;
            }

            //最初に配列をリセットする
            m_triangles.clear();
            m_triangles.reserve(m_lineNum);

            //集中線の位置は実際に中心から線を引いてみて、交差した座標を取得する仕組み
            //交差座標の取得に使う
            OffsetCircular3 cir;

            //半径を長く取る
            //画面より大きければよほどのことがない限り平気なはず
            //図形により大きさの取得法が違うし、十分大きければ特に支障はないので決め打ちします。
            cir.r = 1000000;

            UniformDistribution<double> angleDist(0.0, 2 * Math::Pi);
            UniformDistribution<double> posDist(0.0, m_posRandomness);
            UniformDistribution<double> thickDist(m_minThickness, m_maxThickness);

            for (auto i : step(m_lineNum))
            {
                //極座標を初期化
                const double angle = angleDist(m_rng);
                cir.theta = angle;
                cir.setCenter(Center(m_innerShape));

                //中心線を作る
                Line line(cir.center, cir);

                //内側の座標
                //まずは位置にランダム性を持たせる
                const auto is = m_innerShape.stretched(posDist(m_rng));

                const auto innerintersects = is.intersectsAt(line);

                //内側の図形の中心から線を引き始めているが、図形があまりにも大きいと交差しない
                if (!innerintersects)
                {
                    continue;
                }

                //内側は太さは必要ないので中心線の座標をそのまま使用
                const Vec2 inner = innerintersects->front();

                //外側の中心(基準)となる座標を計算する
                const auto outerintersects = m_outerShape.intersectsAt(line);

                //外側の基準座標を取得できなかったら、その線はスキップして処理は続行
                //並行になっていた場合は…最も遠い点を取得して線を引きたい
                //そもそもInner=Ellipse, Outer=Rectで使っていたら平行にならないはず、
                //その他の図形でもよほど変な形にしなければ平行になる確率は極めて低く、
                //(平行が多発するような図形はとても歪な形で、頑張って線を引いたところでまともな形にならない)
                //実装コスパが悪いので諦めることにします。
                if (!outerintersects || outerintersects->isEmpty())
                {
                    continue;
                }

                const auto outer = outerintersects->front();

                // innerから引いた直線から垂直に、outerの座標から太さの半分ずらした点を2つ生成する処理
                // 90度回す
                const auto rotated = angle + (Math::Pi / 2);

                //ずらす量は太さの半分
                const double r = thickDist(m_rng);

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

                // Triangleの座標として登録
                m_triangles.emplace_back(inner, outerleft, outerright);
            }
        }

        void draw(const ColorF& color = Palette::Black) const
        {
            if (m_isDirty)
            {
                Generate();
                m_isDirty = false;
            }

            for (const auto& triangle : m_triangles)
            {
                triangle.draw(color);
            }
        }
    };
}

// Sample

struct
{
    double value;
    double max;
    double min = 0;
} num, posrandom, minthick, maxthick;

void Main()
{
    DefaultRNGType drt;

    Window::Resize(1280, 720);
    Scene::SetBackground(ColorF(0.97, 0.96, 0.95));

    const Font font(90, Typeface::Default, FontStyle::BoldItalic);

    const Vec2 innerShapeCenter(500, 400);
    const auto innerShape = Ellipse(innerShapeCenter, 200, 100);
    // const auto innerShape = RectF(Arg::center = innerShapeCenter, 200, 100);
    SaturatedLinework linework(innerShape);

    // Ellipse outerShape = innerShape.stretched(400);
    // SaturatedLinework linework(innerShape, outerShape);

    // linework.SetSeed(0);

    num.value = linework.getLineNum();
    posrandom.value = linework.getPosRandomness();
    minthick.value = linework.getMinThickness();
    maxthick.value = linework.getMaxThickness();

    const uint64 seed = linework.getSeed();
    Print(U"seed:", seed);

    num.max = 300;
    posrandom.max = 400;
    minthick.max = 30;
    maxthick.max = 80;

    // Slider GUIのサイズ
    constexpr int32 label = 130;
    constexpr int32 slider = 250;
    // GUIのベース座標
    constexpr int32 x = 20;
    constexpr int32 y = 40;

    while (System::Update())
    {
        font(U"集中線").drawAt(linework.getInnerShape().center, Palette::Black);

        linework.draw(ColorF(0.11));

        if (SimpleGUI::Slider(U"Num{:.0f}"_fmt(num.value), num.value, num.min, num.max, Vec2(x, y), label, slider))
        {
            linework.setLineNum(num.value);
        }

        if (SimpleGUI::Slider(U"PosRand{:.0f}"_fmt(posrandom.value), posrandom.value, posrandom.min, posrandom.max, Vec2(x, y + 40 * 1), label, slider))
        {
            linework.setPosRandomness(posrandom.value);
        }

        if (SimpleGUI::Slider(U"MinThick{:.0f}"_fmt(minthick.value), minthick.value, minthick.min, minthick.max, Vec2(x, y + 40 * 2), label, slider))
        {
            linework.setMinThickness(minthick.value);
            maxthick.value = linework.getMaxThickness();
        }
        if (SimpleGUI::Slider(U"MaxThick{:.0f}"_fmt(maxthick.value), maxthick.value, maxthick.min, maxthick.max, Vec2(x, y + 40 * 3), label, slider))
        {
            linework.setMaxThickness(maxthick.value);
            minthick.value = linework.getMinThickness();
        }

        if (SimpleGUI::Button(U"SeedReset", Vec2(x + 160, y + 40 * 4)))
        {
            linework.setSeed(seed);
        }

        if (SimpleGUI::Button(U"RandomSet", Vec2(x, y + 40 * 4)))
        {
            num.value = Random(num.min, num.max);
            posrandom.value = Random(posrandom.min, posrandom.max);
            minthick.value = Random(minthick.min, minthick.max);
            maxthick.value = Random(minthick.value, maxthick.max);

            linework.setLineNum(static_cast<size_t>(num.value))
                .setThickness(static_cast<double>(minthick.value), static_cast<double>(maxthick.value))
                .setPosRandomness(static_cast<double>(posrandom.value));
        }

        if (SimpleGUI::Button(U"Generate", Vec2(x, y + 40 * 5)))
        {
            linework.Generate();
        }
    }
}
