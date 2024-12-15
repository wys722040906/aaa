#include "statisticsdialog.h"
#include <QApplication>
#include <QScreen>

StatisticsDialog::StatisticsDialog(QWidget *parent)
    : QDialog(parent)
    , m_chartView(new QChartView(this))
    , m_closeButton(new QPushButton("关闭", this))
    , m_chart(new QChart())
    , m_series(new QBarSeries())
    , m_barSet(new QBarSet("楼层使用频率"))
    , m_axisX(new QBarCategoryAxis())
    , m_axisY(new QValueAxis())
{
    setupUI();
    setupChart();
    setWindowTitle("楼层使用统计");
}

void StatisticsDialog::setupUI() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 设置图表视图
    m_chartView->setRenderHint(QPainter::Antialiasing);
    mainLayout->addWidget(m_chartView);

    // 添加关闭按钮
    mainLayout->addWidget(m_closeButton);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);

    // 设置窗口大小为屏幕的60%
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int width = screenGeometry.width() * 0.6;
    int height = screenGeometry.height() * 0.6;
    resize(width, height);
}

void StatisticsDialog::setupChart() {
    // 设置图表标题
    m_chart->setTitle("电梯系统楼层使用统计");
    m_chart->setAnimationOptions(QChart::SeriesAnimations);

    // 设置X轴（楼层）
    QStringList categories;
    for (int i = 1; i <= 14; ++i) {
        categories << QString::number(i) + "楼";
    }
    m_axisX->append(categories);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);

    // 设置Y轴（使用次数）
    m_axisY->setTitleText("使用次数");
    m_axisY->setLabelFormat("%d");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    // 设置柱状图系列
    m_series->append(m_barSet);
    m_chart->addSeries(m_series);
    m_series->attachAxis(m_axisX);
    m_series->attachAxis(m_axisY);

    // 设置图表主题
    m_chart->setTheme(QChart::ChartThemeLight);
    
    // 设置图例可见
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);

    // 将图表添加到视图
    m_chartView->setChart(m_chart);
}

void StatisticsDialog::updateStatistics(const QMap<int, int>& floorStatistics) {
    m_statistics = floorStatistics;
    updateChart();
}

void StatisticsDialog::updateChart() {
    // 清除现有数据
    m_barSet->remove(0, m_barSet->count());

    // 添加新数据
    int maxCount = 0;
    for (int floor = 1; floor <= 14; ++floor) {
        int count = m_statistics.value(floor, 0);
        m_barSet->append(count);
        maxCount = qMax(maxCount, count);
    }

    // 更新Y轴范围
    m_axisY->setRange(0, maxCount * 1.1);  // 留出10%的空间
}

void StatisticsDialog::clearStatistics() {
    m_statistics.clear();
    updateChart();
} 