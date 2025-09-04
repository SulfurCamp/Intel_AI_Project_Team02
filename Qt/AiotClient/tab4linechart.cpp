#include "tab4linechart.h"
#include "ui_tab4linechart.h"

Tab4LineChart::Tab4LineChart(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Tab4LineChart)
{
    ui->setupUi(this);
}

Tab4LineChart::~Tab4LineChart()
{
    delete ui;
}
