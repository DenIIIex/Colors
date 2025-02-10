#include "ballsmodel.h"
#include <QTimer>

BallsModel::BallsModel(QObject *parent) : QAbstractItemModel(parent) {

	initDb();
	for (size_t i = 0; i < _fieldSize; i++) {
		QVector<Ball> row;
		for (size_t j = 0; j < _fieldSize; j++) {
			row.append(Ball(i * _fieldSize + j, false, false,
							static_cast<Colors>(rand() % 4)));
		}
		_balls.append(row);
	}
	newGame(readDb());
}

void BallsModel::newGame(bool isLoad) {
	beginResetModel();
	if (!isLoad) {
		clearDb();
		setScore(0);
		_balls.clear();
		for (size_t i = 0; i < _fieldSize; i++) {
			QVector<Ball> row;
			for (size_t j = 0; j < _fieldSize; j++) {
				row.append(Ball(i * _fieldSize + j, false, false,
								static_cast<Colors>(rand() % 4)));
			}
			_balls.append(row);
		}
	}
	_emptyIndex.clear();
	findEmptyCell();
	endResetModel();
	if (!isLoad)
		turnEnemy();
}

QModelIndex BallsModel::index(int row, int column,
							  const QModelIndex &parent) const {

	if (!hasIndex(row, column, parent))
		return QModelIndex();

	return createIndex(row, column);
}

QModelIndex BallsModel::parent(const QModelIndex &child) const {
	return QModelIndex();
}

QVariant BallsModel::data(const QModelIndex &index, int role) const {

	Ball ball = _balls[index.row()][index.column()];
	switch (role) {
	case Roles::Index:
		return ball.index;
	case Roles::Color:
		switch (ball.color) {
		case C1:
			return QColor("#FFC636");
		case C2:
			return QColor("#00ADA9");
		case C3:
			return QColor("#FFFFFF");
		case C4:
			return QColor("#FF6444");
		default:
			break;
		}
		return ball.color;
	case Roles::IsSelect:
		return ball.isSelect;
	case Roles::isVisible:
		return ball.isVisible;
	default:
		break;
	}
	return QVariant();
}

int BallsModel::score() const { return _score; }

void BallsModel::setScore(int newScore) {
	if (_score == newScore)
		return;
	_score = newScore;
	emit scoreChanged();
}

void BallsModel::checkTable(bool isPlayerTurn) {
	QTimer::singleShot(300, this, [this, isPlayerTurn]() {
		QVector<std::tuple<int, int, int, int>> b;
		int lInd = 0;
		int rInd = 0;
		int curLen = 1;
		for (size_t row = 0; row < _fieldSize; row++) {
			lInd = 0;
			rInd = 0;
			curLen = 1;
			for (size_t i = 1; i < _fieldSize; i++) {
				if ((_balls[row][i].isVisible &&
					 _balls[row][i - 1].isVisible) &&
					_balls[row][i].color == _balls[row][i - 1].color) {
					curLen++;
					if (curLen >= 5) {
						rInd = i;
						lInd = rInd - curLen + 1;
					}
				} else {
					curLen = 1;
				}
			}
			if (rInd - lInd >= 4) {
				b.push_back(std::make_tuple(row, -1, lInd, rInd));
			}
		}

		for (size_t col = 0; col < _fieldSize; col++) {
			lInd = 0;
			rInd = 0;
			curLen = 1;
			for (size_t i = 1; i < _fieldSize; i++) {
				if ((_balls[i][col].isVisible &&
					 _balls[i - 1][col].isVisible) &&
					_balls[i][col].color == _balls[i - 1][col].color) {
					curLen++;
					if (curLen >= 5) {
						rInd = i;
						lInd = rInd - curLen + 1;
					}
				} else {
					curLen = 1;
				}
			}
			if (rInd - lInd >= 4) {
				b.push_back(std::make_tuple(-1, col, lInd, rInd));
			}
		}
		for (auto t : b) {
			auto l = std::get<2>(t);
			auto r = std::get<3>(t);
			if (std::get<0>(t) == -1) {
				auto col = std::get<1>(t);
				for (size_t i = l; i <= r; i++) {
					_balls[i][col].isVisible = false;
					_emptyIndex.push_back(_balls[i][col].index);
					emit dataChanged(index(l, col, QModelIndex()),
									 index(r, col, QModelIndex()));
				}
				setScore(_score + r - l + 1);
			} else {
				auto row = std::get<0>(t);
				for (size_t i = l; i <= r; i++) {
					_balls[row][i].isVisible = false;
					_emptyIndex.push_back(_balls[row][i].index);
					emit dataChanged(index(row, l, QModelIndex()),
									 index(row, r, QModelIndex()));
				}
				setScore(_score + r - l + 1);
			}
		}
		if (isPlayerTurn) {
			turnEnemy();
		}
	});
}

void BallsModel::initDb() {
	_db = QSqlDatabase::addDatabase("QSQLITE");
	_db.setDatabaseName("lines.db");
	_db.open();
}

void BallsModel::saveDb() {
	if (!_db.isValid()) {
		initDb();
	}
	QByteArray bArray;
	QDataStream stream(&bArray, QIODevice::WriteOnly);
	for (size_t i = 0; i < _fieldSize; i++) {
		for (size_t j = 0; j < _fieldSize; j++) {
			stream << _balls[i][j];
		}
	}
	if (!_db.open()) {
		qDebug() << "Open error: " << _db.lastError().text();
	}

	QSqlQuery query;
	if (!query.exec("create table if not exists session(data "
					"blob,score int)")) {
		qDebug() << "Query error create: " << _db.lastError().text();
		_db.close();
		return;
	}
	query.prepare("replace into session (data,score) values (:data,:score)");
	query.bindValue(":data", bArray);
	query.bindValue(":score", _score);
	if (!query.exec()) {
		qDebug() << "Query error replace: " << _db.lastError().text();
		_db.close();
		return;
	}
	_db.close();
}

bool BallsModel::readDb() {
	QSqlQuery query;
	if (!query.exec("select data from session order by rowid desc limit 1")) {
		qDebug() << "Query error select data: " << _db.lastError().text();
	}
	if (query.next()) {
		QByteArray bArray = query.value(0).toByteArray();
		QDataStream stream(&bArray, QIODevice::ReadOnly);
		for (size_t i = 0; i < _fieldSize; i++) {
			for (size_t j = 0; j < _fieldSize; j++) {
				stream >> _balls[i][j];
			}
		}
	} else {
		return false;
	}
	if (!query.exec("select score from session order by rowid desc limit 1")) {
		qDebug() << "Query error select score: " << _db.lastError().text();
	}
	if (query.next()) {
		setScore(query.value(0).toInt());
	} else {
		return false;
	}
	return true;
}

void BallsModel::clearDb() {
	if (!_db.open()) {
		qDebug() << "Open error: " << _db.lastError().text();
	}
	QSqlQuery query;

	if (!query.exec("delete from session")) {
		qDebug() << "Query error delete: " << _db.lastError().text();
	}
	_db.close();
}

void BallsModel::printModel() {
	qDebug() << "\\\\\\\\\\\\\\";
	for (size_t i = 0; i < _fieldSize; i++) {
		QString str;
		for (size_t j = 0; j < _fieldSize; j++) {
			str.push_back(_balls[i][j].isVisible ? "1" : "0");
		}
		qDebug() << str;
	}
	qDebug() << "//////////////";
}

void BallsModel::checkTurn(int ind, int x, int y, QColor col) {
	QPoint p = getRowColbyInd(ind);
	auto c1 = &_balls[p.x()][p.y()];
	if (_selInd >= 0) {
		if (_selInd == ind) {
			return;
		}
		QPoint pp = getRowColbyInd(_selInd);
		auto c2 = &_balls[pp.x()][pp.y()];
		if (c1->isVisible) {
			c1->isSelect = true;
			c2->isSelect = false;
			_selInd = ind;
			_sel = QPoint(x, y);
			_selCol = col;
		} else {
			std::swap(c1->color, c2->color);
			_fake = p;
			c2->isVisible = false;
			_emptyIndex.removeOne(ind);
			_emptyIndex.push_back(_selInd);
			c2->isSelect = false;
			emit moveBall(_selCol, _sel.x(), _sel.y(), x, y);
			_selInd = -1;
			_sel = QPoint(-1, -1);
			checkTable(true);
		}

		emit dataChanged(index(pp.x(), pp.y(), QModelIndex()),
						 index(pp.x(), pp.y(), QModelIndex()));
	} else {
		if (c1->isVisible) {
			c1->isSelect = true;
			_selInd = ind;
			_sel = QPoint(x, y);
			_selCol = col;
		}
	}
	emit dataChanged(index(p.x(), p.y(), QModelIndex()),
					 index(p.x(), p.y(), QModelIndex()));
}

void BallsModel::setBallVisible() {
	_balls[_fake.x()][_fake.y()].isVisible = true;
	emit dataChanged(index(_fake.x(), _fake.y(), QModelIndex()),
					 index(_fake.x(), _fake.y(), QModelIndex()));
}

void BallsModel::findEmptyCell() {
	for (size_t i = 0; i < _fieldSize; i++) {
		for (size_t j = 0; j < _fieldSize; j++) {
			if (!_balls[i][j].isVisible)
				_emptyIndex.push_back(_balls[i][j].index);
		}
	}
}

void BallsModel::turnEnemy() {
	QTimer::singleShot(100, this, [this]() {
		if (_emptyIndex.size() < 4) {
			emit gameOver();
			clearDb();
			return;
		}
		for (size_t i = 0; i < 3; i++) {
			int ind = rand() % _emptyIndex.size();
			QPoint p = getRowColbyInd(_emptyIndex[ind]);
			_balls[p.x()][p.y()].color = static_cast<Colors>(rand() % 4);
			_balls[p.x()][p.y()].isVisible = true;
			_emptyIndex.removeOne(_emptyIndex[ind]);

			emit dataChanged(index(p.x(), p.y(), QModelIndex()),
							 index(p.x(), p.y(), QModelIndex()));
			checkTable(false);
		}
		saveDb();
	});
}
QPoint BallsModel::getRowColbyInd(int ind) {
	return QPoint(floor(ind / _fieldSize), ind % _fieldSize);
}
