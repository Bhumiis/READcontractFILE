#include <QApplication>
#include <QDebug>
#include <QMainWindow>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QHeaderView>
#include <algorithm>
#include <vector>

struct StrikeData {
    QString symbol;
    int itm;
    int atm;
    int otm;
    int total() const { return itm + atm + otm; }
};

// Helper to read CSV
std::vector<StrikeData> readCSV(const QString& filePath) {
    std::vector<StrikeData> data;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return data;

    QTextStream in(&file);
    bool firstLine = true;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (firstLine) { firstLine = false; continue; } // skip header
        auto fields = line.split(",");
        if (fields.size() < 7) continue;

        StrikeData sd;
        sd.symbol = fields[0].trimmed();
        sd.itm = fields[4].toInt();
        sd.atm = fields[5].toInt();
        sd.otm = fields[6].toInt();
        data.push_back(sd);
    }
    return data;
}

// Populate QTableWidget
void populateTable(QTableWidget* table, const std::vector<StrikeData>& data) {
    table->setRowCount(data.size());
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"Symbol", "ITM", "ATM", "OTM", "Total"});
    table->horizontalHeader()->setStretchLastSection(true);

    for (int i = 0; i < data.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(data[i].symbol));
        table->setItem(i, 1, new QTableWidgetItem(QString::number(data[i].itm)));
        table->setItem(i, 2, new QTableWidgetItem(QString::number(data[i].atm)));
        table->setItem(i, 3, new QTableWidgetItem(QString::number(data[i].otm)));
        table->setItem(i, 4, new QTableWidgetItem(QString::number(data[i].total())));
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QMainWindow window;

    QWidget* central = new QWidget(&window);
    QVBoxLayout* layout = new QVBoxLayout(central);

    QTableWidget* topTable = new QTableWidget();
    QTableWidget* bottomTable = new QTableWidget();

    layout->addWidget(topTable);
    layout->addWidget(bottomTable);

    window.setCentralWidget(central);
    window.resize(600, 400);
    window.setWindowTitle("Top/Bottom Strike Symbols");

    // Read CSV
    QString csvPath = "C:/Users/Administrator/Downloads/NSE_FO_SosScheme.csv";
    qDebug() << "Trying to read CSV from:" << csvPath;

    auto allData = readCSV(csvPath);
    if (allData.empty()) {
        topTable->setRowCount(1);
        topTable->setColumnCount(1);
        topTable->setItem(0,0,new QTableWidgetItem("No data found!"));
        window.show();
        return a.exec();
    }

    // Sort descending by total strikes for Top 5
    auto topData = allData;
    std::sort(topData.begin(), topData.end(),
              [](const StrikeData& a, const StrikeData& b){ return a.total() > b.total(); });
    if (topData.size() > 5) topData.resize(5);

    // Sort ascending by total strikes for Bottom 5
    auto bottomData = allData;
    std::sort(bottomData.begin(), bottomData.end(),
              [](const StrikeData& a, const StrikeData& b){ return a.total() < b.total(); });
    if (bottomData.size() > 5) bottomData.resize(5);

    populateTable(topTable, topData);
    populateTable(bottomTable, bottomData);

    window.show();
    return a.exec();
}
