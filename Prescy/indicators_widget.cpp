#include "indicators_widget.hpp"

#include <Engine/exception.hpp>

#include <QTabWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QLabel>

namespace prescybase {

IndicatorsWidget::IndicatorsWidget(const std::vector<prescyengine::StockIndicator>& indicators, QWidget *parent) :
    QWidget(parent),
    _tabWidget{this}
{
    auto addIndicatorButton= new QPushButton("Add Indicator", this);
    addIndicatorButton->setMaximumWidth(140);
    addIndicatorButton->setStatusTip("Add a new indicator");
    connect(addIndicatorButton, &QPushButton::clicked, this, [this](){
        addPage(prescyengine::StockIndicator{}, _tabWidget.count());
    });

    auto removeIndicatorButton= new QPushButton("Remove Indicator", this);
    removeIndicatorButton->setMaximumWidth(140);
    removeIndicatorButton->setStatusTip("Remove the selected indicator");
    connect(removeIndicatorButton, &QPushButton::clicked, this, [this]() {
        auto page = _tabWidget.currentWidget();
        if (page) {
            emit indicatorRemoved(_indicators[page].name);
            _indicators.erase(page);
            _tabWidget.removeTab(_tabWidget.currentIndex());
        }
    });

    auto layout = new QGridLayout();
    layout->addWidget(addIndicatorButton, 0, 0, 1, 1);
    layout->addWidget(removeIndicatorButton, 0, 1, 1, 1);
    layout->addWidget(&_tabWidget, 1, 0, 1, 3);
    this->setLayout(layout);
    this->setMaximumHeight(300);

    for (const auto& indicator : indicators) {
        addPage(indicator, _tabWidget.count());
    }
    _tabWidget.setCurrentIndex(0);
}

void IndicatorsWidget::addPage(const prescyengine::StockIndicator &indicator, int index) {
    auto page = new QWidget(this);
    auto pageLayout = new QVBoxLayout(page);
    pageLayout->addWidget(new QLabel("Name"));
    auto nameEdit = new QLineEdit(page);
    nameEdit->setText(QString::fromStdString(indicator.name));
    pageLayout->addWidget(nameEdit);

    auto luaLabel = new QLabel("<a href=\"https://www.lua.org/manual/5.4/\">Lua 5.4 Manual</a>");
    luaLabel->setTextFormat(Qt::RichText);
    luaLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    luaLabel->setOpenExternalLinks(true);
    pageLayout->addWidget(luaLabel);

    pageLayout->addWidget(new QLabel("Expression"));
    auto expressionEdit = new QPlainTextEdit(page);
    expressionEdit->setPlainText(QString::fromStdString(indicator.expression));
    pageLayout->addWidget(expressionEdit);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Save, page);
    connect(buttonBox, &QDialogButtonBox::accepted, page, [this, nameEdit, expressionEdit]() {
        if (!nameEdit->text().isEmpty() && !expressionEdit->toPlainText().isEmpty()) {
            auto indicator = prescyengine::StockIndicator(nameEdit->text().toStdString(),
                                                          expressionEdit->toPlainText().toStdString());
            auto index = _tabWidget.currentIndex();
            _tabWidget.removeTab(_tabWidget.currentIndex());
            addPage(indicator, index);
            emit indicatorAddedOrUpdated(indicator);
        }
    });
    pageLayout->addWidget(buttonBox);
    page->setLayout(pageLayout);
    _tabWidget.addTab(page, QString::fromStdString(indicator.name));
    _tabWidget.setCurrentIndex(index);
    _indicators[page] = indicator;
}

}
