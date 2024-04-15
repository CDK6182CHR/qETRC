#pragma once
#include <QDialog>
#include <vector>


class TrainPath;
class QTableView;
class PathListReadModel;
class TrainPathCollection;
/**
 * The dialog for selecting (possibly multiple) trainpaths.
 * Also provide static functions.
 */
class SelectPathDialog : public QDialog
{
    Q_OBJECT
    PathListReadModel* const model;
    QTableView* table;
public:
    SelectPathDialog(const QString& prompt, QWidget* parent=nullptr);
    std::vector<TrainPath*> selectedPaths();

    /**
     * Return the selected indexes of the input paths.
     * The result is sorted.
     */
    std::vector<int> selectedIndexes();

    static std::vector<TrainPath*>
        getPaths(QWidget* parent, const QString& prompt, std::vector<TrainPath*> paths, bool single=false);

    static std::vector<int>
        getPathIndexes(QWidget* parent, const QString& prompt, std::vector<TrainPath*> paths, bool multi=true);

    auto* getModel(){return model;}

private:
    void initUI(const QString& prompt);

private slots:
    void actSelectAll();
};

