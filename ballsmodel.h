#include <QAbstractItemModel>
#include <QColor>
#include <QPoint>
#include <QtSql>

class BallsModel : public QAbstractItemModel {
	Q_OBJECT
	Q_PROPERTY(int score READ score WRITE setScore NOTIFY scoreChanged FINAL)

	enum Roles { Index = Qt::UserRole + 1, IsSelect, isVisible, Color };

	enum Colors { C1 = 0, C2, C3, C4 };

	struct Ball {
		Ball(int ind, bool isSel, bool isVis, Colors col) {
			index = ind;
			isSelect = isSel;
			isVisible = isVis;
			color = col;
		}
		Ball(){};
		friend QDataStream &operator<<(QDataStream &out, const Ball &ball) {
			out << ball.index << ball.color << ball.isSelect << ball.isVisible;
			return out;
		}
		friend QDataStream &operator>>(QDataStream &in, Ball &ball) {
			in >> ball.index >> ball.color >> ball.isSelect >> ball.isVisible;
			return in;
		}
		int index;
		bool isVisible;
		bool isSelect;
		Colors color;
	};

  public:
	BallsModel(QObject *parent = nullptr);
	~BallsModel(){};

	QModelIndex index(int row, int column, const QModelIndex &parent) const;
	QModelIndex parent(const QModelIndex &child) const;
	int rowCount(const QModelIndex &parent) const { return _fieldSize; };
	int columnCount(const QModelIndex &parent) const { return _fieldSize; };
	QVariant data(const QModelIndex &index, int role) const;

	QHash<int, QByteArray> roleNames() const {
		return {
			{Index, QByteArrayLiteral("bInd")},
			{IsSelect, QByteArrayLiteral("bSel")},
			{isVisible, QByteArrayLiteral("bVis")},
			{Color, QByteArrayLiteral("bColor")},
		};
	};

	int score() const;
	void setScore(int newScore);
	Q_INVOKABLE void checkTurn(int ind, int x, int y, QColor col);
	Q_INVOKABLE void setBallVisible();
	Q_INVOKABLE void newGame(bool isLoad);

  signals:
	void scoreChanged();
	void moveBall(QColor color, int x, int y, int nX, int nY);
	void gameOver();

  private:
	const int _fieldSize = 9;

	void turnEnemy();
	void findEmptyCell();
	void printModel();
	void checkTable(bool isPlayerTurn);
	void initDb();
	void saveDb();
	bool readDb();
	void clearDb();

	int _fakeRow = -1;
	int _fakeCol = -1;
	int _score = 0;
	int _selInd = -1;
	int _selX = -1;
	int _selY = -1;
	QColor _selCol;
	QPoint _sel;
	QPoint _fake;
	QVector<QVector<Ball>> _balls;
	QVector<int> _emptyIndex;
	QPoint getRowColbyInd(int ind);
	QSqlDatabase _db;
};
