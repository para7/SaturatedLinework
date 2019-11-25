#include <Siv3D.hpp>

namespace s3d
{
    template <class InnerShape = Ellipse, class URNG = DefaultRNGType>
    class SaturatedLinework
    {
    private:

        // 内側の図形
        InnerShape m_innerShape;

        // 内側の図形
        Rect m_outerRect;

        // 線の数
        size_t m_lineCount = 70;

        // 線の太さの最小幅
        double m_minThickness = 7.0;

        // 線の太さの最大幅
        double m_maxThickness = 10.0;

        // 出現位置のランダム幅
        double m_targetOffsetRange = 0.0;

        // 乱数エンジンのシード値
        uint64 m_seed = 12345;

        // 乱数エンジン
        mutable URNG m_rng;

        // 集中線の中身
        mutable Array<Triangle> m_triangles;

        mutable bool m_isDirty = true;

        // innerShape　が outerShape に含まれているか簡単なチェック
        template <class _InnerShape>
        [[nodiscard]] static bool IsValid(const _InnerShape& innerShape, const Rect& outerShape)
        {
            // [Siv3D ToDo] より厳密に
            return outerShape.contains(Geometry2D::Center(innerShape));
        }

    public:

        SaturatedLinework()
            : m_innerShape(Ellipse(Scene::CenterF(), 200, 100))
            , m_outerRect(Scene::Rect())
        {
            setSeed(RandomUint64());
        }

        explicit SaturatedLinework(const InnerShape& innerShape)
            : m_innerShape(innerShape)
            , m_outerRect(Scene::Rect())
        {
            setSeed(RandomUint64());
        }

        SaturatedLinework(const InnerShape& innerShape, const Rect& outerShape)
            : m_innerShape(innerShape)
            , m_outerRect(outerShape)
        {
            setSeed(RandomUint64());
        }

        SaturatedLinework& setInnerShape(const InnerShape& innerShape)
        {
            if (innerShape != m_innerShape)
            {
                m_innerShape = innerShape;
                m_isDirty = true;
            }

            return *this;
        }

        [[nodiscard]] const InnerShape& getInnerShape() const noexcept
        {
            return m_innerShape;
        }

        SaturatedLinework& setOuterRect(const Rect& outerRect)
        {
            if (outerRect != m_outerRect)
            {
                m_outerRect = outerRect;
                m_isDirty = true;
            }

            return *this;
        }

        [[nodiscard]] const Rect& getOuterRect() const noexcept
        {
            return m_outerRect;
        }

        SaturatedLinework& setLineCount(size_t lineCount)
        {
            if (lineCount != m_lineCount)
            {
                m_lineCount = lineCount;
                m_isDirty = true;
            }

            return *this;
        }

        [[nodiscard]] size_t getLineCount() const noexcept
        {
            return m_lineCount;
        }

        SaturatedLinework& setMinThickness(double minThickness)
        {
            if (minThickness < 0.0)
            {
                throw Error(U"SaturatedLinework::setMinThickness(): minThickness < 0.0");
            }

            if (m_maxThickness < minThickness)
            {
                throw Error(U"SaturatedLinework::setMinThickness(): m_maxThickness < minThickness");
            }

            if (minThickness != m_minThickness)
            {
                m_minThickness = minThickness;
                m_isDirty = true;
            }

            return *this;
        }

        [[nodiscard]] double getMinThickness() const noexcept
        {
            return m_minThickness;
        }

        SaturatedLinework& setMaxThickness(double maxThickness)
        {
            if (maxThickness < m_minThickness)
            {
                throw Error(U"SaturatedLinework::setMaxThickness(): maxThickness < m_minThickness");
            }

            if (m_maxThickness != maxThickness)
            {
                m_maxThickness = maxThickness;
                m_isDirty = true;
            }

            return *this;
        }

        [[nodiscard]] double getMaxThickness() const noexcept
        {
            return m_maxThickness;
        }

        SaturatedLinework& setThickness(double minThickness, double maxThickness)
        {
            if (minThickness < 0.0)
            {
                throw Error(U"SaturatedLinework::setThickness(): minThickness < 0.0");
            }

            if (maxThickness < m_minThickness)
            {
                throw Error(U"SaturatedLinework::setThickness(): maxThickness < m_minThickness");
            }

            if (minThickness != m_minThickness)
            {
                m_minThickness = minThickness;
                m_isDirty = true;
            }

            if (maxThickness != m_maxThickness)
            {
                m_maxThickness = maxThickness;
                m_isDirty = true;
            }

            return *this;
        }

        SaturatedLinework& setTargetOffsetRange(double targetOffsetRange)
        {
            if (targetOffsetRange != m_targetOffsetRange)
            {
                m_targetOffsetRange = targetOffsetRange;
                m_isDirty = true;
            }
            return *this;
        }

        [[nodiscard]] double getTargetOffsetRange() const noexcept
        {
            return m_targetOffsetRange;
        }

        SaturatedLinework& setSeed(uint64 seed)
        {
            m_seed = seed;
            m_rng.seed(seed);
            m_isDirty = true;
            return *this;
        }

        [[nodiscard]] uint64 getSeed() const noexcept
        {
            return m_seed;
        }

        // 集中線を生成する
        void generate() const
        {
            const int32 offset = static_cast<int32>(m_maxThickness / 2);
            const Rect outerRect = m_outerRect.stretched(offset);

            // 内部の図形が外部の図形に包まれてない場合はエラー
            // [ToDo] より厳密に
            if (!IsValid(m_innerShape, outerRect))
            {
                return;
            }

            // 最初に配列をリセットする
            m_triangles.clear();
            m_triangles.reserve(m_lineCount);

            // 集中線の位置は実際に中心から線を引いてみて、交差した座標を取得する仕組み
            // 交差座標の取得に使う
            OffsetCircular3 cir;
            cir.setCenter(Geometry2D::Center(m_innerShape));

            // 半径を長く取る
            // 画面より大きければよほどのことがない限り平気なはず
            // 図形により大きさの取得法が違うし、十分大きければ特に支障はないので決め打ちします。
            cir.r = 1000000;

            UniformDistribution<double> angleDist(0.0, 2_pi);
            UniformDistribution<double> posDist(0.0, m_targetOffsetRange);
            UniformDistribution<double> thickDist(m_minThickness, m_maxThickness);

            for (size_t i = 0; i < m_lineCount; ++i)
            {
                // 生成角度を設定
                const double angle = angleDist(m_rng);
                cir.theta = angle;

                // 角度に沿った線を作る
                const Line line(cir.center, cir);

                // 内側の座標を計算する
                // まずは位置にランダム性を持たせる
                const auto is = m_innerShape.stretched(posDist(m_rng));

                const auto innerIntersects = is.intersectsAt(line);

                // 内側の図形の中心から線を引き始めているが、図形があまりにも大きいと交差しないので注意
                if (!innerIntersects || innerIntersects->isEmpty())
                {
                    continue;
                }

                // 内側は太さは必要ないので中心線の座標をそのまま使用
                const Vec2 inner = innerIntersects->front();

                // 外側の中心(基準)となる座標を計算する そのまま使うわけではない
                const auto outerIntersects = outerRect.intersectsAt(line);

                // 外側の基準座標を取得できなかったら、その線はスキップして処理は続行
                // 並行になっていた場合は…最も遠い点を取得して線を引きたい
                // そもそもInner=Ellipse, Outer=Rectで使っていたら平行にならないはず。
                // その他の図形でもよほど変な形にしなければ平行になる確率は極めて低く
                // (平行が多発するような図形はとても歪な形で、頑張って線を引いたところでまともな形にならない)
                // 実装コスパが悪いので諦める方が無難と思われる。
                if (!outerIntersects || outerIntersects->isEmpty())
                {
                    continue;
                }

                const Vec2 outer = outerIntersects->front();

                // innerから引いた直線から垂直に、outerの座標から太さの半分ずらした点を2つ生成する処理
                // 90度回す
                const double rotated = angle + 90_deg;

                // ずらす量
                // 左右に広げるので、ここでの量は半分
                const double r = thickDist(m_rng) / 2;

                // 単位ベクトル
                const Vec2 v(cos(rotated), sin(rotated));
                const Vec2 outerOffset = v * r;

                // 左右にずらした座標を作る
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
                generate();
                m_isDirty = false;
            }

            for (const Triangle& triangle : m_triangles)
            {
                triangle.draw(color);
            }
        }
    };
}
