#pragma once
#include <QComboBox>
#include <QList>
#include <memory>


class DiagramPage;

/**
 * 2024.04.15  A combo box for selecting diagram page.
 * Currently, the signal for changing selection is not implemented.
 */
class SelectPageCombo : public QComboBox
{
	Q_OBJECT;
	const QList<std::shared_ptr<DiagramPage>>& pages;
public:
	SelectPageCombo(const QList<std::shared_ptr<DiagramPage>>& pages, QWidget* parent = nullptr);

	std::shared_ptr<DiagramPage> currentPage();

	static std::shared_ptr<DiagramPage> dialogGetPage(const QList<std::shared_ptr<DiagramPage>>& pages, QWidget* parent,
		const QString& title, const QString& prompt);
private:
	void initData();

};