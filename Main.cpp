
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

    template <class InnerShape = Ellipse, class URNG = DefaultRNGType>
    class SaturatedLinework
    {
    private:
        //内側の図形
        InnerShape m_innerShape;

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
            if (innerShape != m_innerShape)
            {
                m_innerShape = innerShape;
                m_isDirty = true;
            }
            return *this;
        }

        const InnerShape& getInnerShape() const noexcept
        {
            return m_innerShape;
        }

        SaturatedLinework& setLineNum(size_t lineNum)
        {
            if (lineNum != m_lineNum)
            {
                m_lineNum = lineNum;
                m_isDirty = true;
            }
            return *this;
        }

        size_t getLineNum() const noexcept
        {
            return m_lineNum;
        }

        SaturatedLinework& setMinThickness(double minThickness)
        {
            // minThickness = Max(0.0, minThickness);
            // minThickness = Min(m_maxThickness, minThickness);

            assert(0.0 <= minThickness);
            assert(minThickness <= m_maxThickness);

            if (m_minThickness != minThickness)
            {
                m_minThickness = minThickness;
                m_maxThickness = Max(m_minThickness, m_maxThickness);
                m_isDirty = true;
            }
            return *this;
        }

        double getMinThickness() const noexcept
        {
            return m_minThickness;
        }

        SaturatedLinework& setMaxThickness(double maxThickness)
        {
            // maxThickness = Max(m_minThickness, maxThickness);
            assert(m_minThickness <= maxThickness);

            if (m_maxThickness != maxThickness)
            {
                m_maxThickness = maxThickness;
                m_minThickness = Min(m_minThickness, m_maxThickness);
                m_isDirty = true;
            }
            return *this;
        }

        double getMaxThickness() const noexcept
        {
            return m_maxThickness;
        }

        SaturatedLinework& setThickness(double minThickness, double maxThickness)
        {
            assert(0.0 <= minThickness);
            assert(minThickness <= maxThickness);
            if (m_minThickness != minThickness)
            {
                m_minThickness = minThickness;
                m_isDirty = true;
            }
            if (m_maxThickness != maxThickness)
            {
                m_maxThickness = maxThickness;
                m_isDirty = true;
            }
            return *this;
        }

        SaturatedLinework& setPosRandomness(double posRandomness)
        {
            if (m_posRandomness != posRandomness)
            {
                m_posRandomness = posRandomness;
                m_isDirty = true;
            }
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
        {
            m_seed = Random(std::numeric_limits<uint64>::max());
            setSeed(m_seed);
        }

        explicit SaturatedLinework(const InnerShape& innerShape)
            : m_innerShape(innerShape)
        {
            m_seed = Random(std::numeric_limits<uint64>::max());
            setSeed(m_seed);
        }

    public:  // func
        //集中線を生成する
        void Generate() const
        {
            Generate(Scene::Rect().stretched(m_maxThickness / 2, m_maxThickness / 2));
        }

        void Generate(const Rect& outerRect) const
        {
            // 内部の図形が外部の図形に包まれてない場合はエラー
            // 描画がおかしくなるだけで処理的に致命的な何かが起きるわけでもないので、
            // 実はエラー出さなくてもいい？
            if (!IsValid(m_innerShape, outerRect))
            {
                return;
            }

            //最初に配列をリセットする
            m_triangles.clear();
            m_triangles.reserve(m_lineNum);

            //集中線の位置は実際に中心から線を引いてみて、交差した座標を取得する仕組み
            //交差座標の取得に使う
            OffsetCircular3 cir;
            cir.setCenter(Center(m_innerShape));

            //半径を長く取る
            //画面より大きければよほどのことがない限り平気なはず
            //図形により大きさの取得法が違うし、十分大きければ特に支障はないので決め打ちします。
            cir.r = 1000000;

            UniformDistribution<double> angleDist(0.0, 2 * Math::Pi);
            UniformDistribution<double> posDist(0.0, m_posRandomness);
            UniformDistribution<double> thickDist(m_minThickness, m_maxThickness);

            for (auto i : step(m_lineNum))
            {
                //生成角度を設定
                const double angle = angleDist(m_rng);
                cir.theta = angle;

                //角度に沿った線を作る
                const Line line(cir.center, cir);

                //内側の座標を計算する
                //まずは位置にランダム性を持たせる
                const auto is = m_innerShape.stretched(posDist(m_rng));

                const auto innerIntersects = is.intersectsAt(line);

                //内側の図形の中心から線を引き始めているが、図形があまりにも大きいと交差しないので注意
                if (!innerIntersects || innerIntersects->isEmpty())
                {
                    continue;
                }

                //内側は太さは必要ないので中心線の座標をそのまま使用
                const Vec2 inner = innerIntersects->front();

                //外側の中心(基準)となる座標を計算する そのまま使うわけではない
                const auto outerIntersects = outerRect.intersectsAt(line);

                //外側の基準座標を取得できなかったら、その線はスキップして処理は続行
                //並行になっていた場合は…最も遠い点を取得して線を引きたい
                //そもそもInner=Ellipse, Outer=Rectで使っていたら平行にならないはず。
                //その他の図形でもよほど変な形にしなければ平行になる確率は極めて低く
                //(平行が多発するような図形はとても歪な形で、頑張って線を引いたところでまともな形にならない)
                //実装コスパが悪いので諦める方が無難と思われる。
                if (!outerIntersects || outerIntersects->isEmpty())
                {
                    continue;
                }

                const Vec2 outer = outerIntersects->front();

                // innerから引いた直線から垂直に、outerの座標から太さの半分ずらした点を2つ生成する処理
                // 90度回す
                const double rotated = angle + (Math::Pi / 2);

                //ずらす量
                //左右に広げるので、ここでの量は半分
                const double r = thickDist(m_rng) / 2;

                //単位ベクトル
                const Vec2 v(cos(rotated), sin(rotated));

                const Vec2 outerOffset = v * r;

                //このあたりも極座標にしようと書き換えてみたところ、計算ミスでぶっ飛んでいったので
                //特に動作も計算コストも問題ありませんし、そのままにしておきます。

                //左右にずらした座標を作る
                // 180度にずらすので、-+で対処可能
                const Vec2 outerLeft = outer + outerOffset;
                const Vec2 outerRight = outer - outerOffset;

                // Triangleの座標として登録
                m_triangles.emplace_back(inner, outerLeft, outerRight);
            }
        }

        void draw(const ColorF& color = Palette::Black) const
        {
            if (m_isDirty)
            {
                Generate();
                m_isDirty = false;
            }

            for (const Triangle& triangle : m_triangles)
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
} num, posRandom, minThick, maxThick;

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
    posRandom.value = linework.getPosRandomness();
    minThick.value = linework.getMinThickness();
    maxThick.value = linework.getMaxThickness();

    const uint64 seed = linework.getSeed();
    Print(U"seed:", seed);

    num.max = 300;
    posRandom.max = 400;
    minThick.max = 50;
    maxThick.max = 120;

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

        SimpleGUI::Slider(U"Num{:.0f}"_fmt(num.value), num.value, num.min, num.max, Vec2(x, y), label, slider);
        SimpleGUI::Slider(U"PosRand{:.0f}"_fmt(posRandom.value), posRandom.value, posRandom.min, posRandom.max, Vec2(x, y + 40 * 1), label, slider);

        if (SimpleGUI::Slider(U"MinThick{:.0f}"_fmt(minThick.value), minThick.value, minThick.min, minThick.max, Vec2(x, y + 40 * 2), label, slider))
        {
            maxThick.value = Max(maxThick.value, minThick.value);
        }

        if (SimpleGUI::Slider(U"MaxThick{:.0f}"_fmt(maxThick.value), maxThick.value, maxThick.min, maxThick.max, Vec2(x, y + 40 * 3), label, slider))
        {
            minThick.value = Min(maxThick.value, minThick.value);
        }

        linework.setLineNum(num.value);
        linework.setPosRandomness(posRandom.value);
        linework.setThickness(minThick.value, maxThick.value);

        if (SimpleGUI::Button(U"SeedReset", Vec2(x + 160, y + 40 * 4)))
        {
            linework.setSeed(seed);
        }

        if (SimpleGUI::Button(U"RandomSet", Vec2(x, y + 40 * 4)))
        {
            num.value = Random(num.min, num.max);
            posRandom.value = Random(posRandom.min, posRandom.max);
            minThick.value = Random(minThick.min, minThick.max);
            maxThick.value = Random(minThick.value, maxThick.max);

            linework.setLineNum(static_cast<size_t>(num.value))
                .setThickness(static_cast<double>(minThick.value), static_cast<double>(maxThick.value))
                .setPosRandomness(static_cast<double>(posRandom.value));
        }

        if (SimpleGUI::Button(U"Generate", Vec2(x, y + 40 * 5)))
        {
            linework.Generate();
        }
    }
}
