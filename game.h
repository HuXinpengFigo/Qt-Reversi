#include <stdio.h>
#include <time.h>
#ifndef GAME_H
#define GAME_H

#define BOARD_SIZE 8 // 棋盘格数
#define STONE_SIZE 65 // 棋子显示大小

#include <QApplication>
#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QToolButton>
#include <QPushButton>
#include <QIcon>
#include <QSignalMapper>
#include <QObject>
#include <QMessageBox>

typedef enum { null, black, white } piece;

class Game : public QObject {
	Q_OBJECT
	public:
		Game(QLabel **guiBoard_i, QLabel *guiTurn_i,
				QPixmap *guiImgStoneW_i, QPixmap *guiImgStoneB_i,
				QPixmap *guiImgStoneWa_i, QPixmap *guiImgStoneBa_i,
				QPixmap *guiImgStoneWs_i, QPixmap *guiImgStoneBs_i,
				QPixmap *guiImgStoneN_i) {
            /* 存图片 */
			guiBoard = guiBoard_i;
			guiTurn = guiTurn_i;
			guiImgStoneW = guiImgStoneW_i;
			guiImgStoneB = guiImgStoneB_i;
			guiImgStoneWa = guiImgStoneWa_i;
			guiImgStoneBa = guiImgStoneBa_i;
			guiImgStoneWs = guiImgStoneWs_i;
			guiImgStoneBs = guiImgStoneBs_i;
			guiImgStoneN = guiImgStoneN_i;
            /* 棋盘归零 */
			for (int i=0; i<BOARD_SIZE*BOARD_SIZE; i++) {
				boardData[i] = null;
			}
			/* 初始化 */
			isAI_W = false;
			isAI_B = false;
			hint = false;
			srand(time(NULL));
		}

        /* 取得棋盘某格的资料 */
		piece getBoardData(int x, int y) {
			if (x > BOARD_SIZE-1 || y > BOARD_SIZE-1 || x < 0 || y < 0) return null;
			return boardData[x+y*BOARD_SIZE];
		}

		/* User 落棋 */
		void userDrop(int x, int y) {
			if (nxtMove == black && isAI_B) return;
			if (nxtMove == white && isAI_W) return;
			if (moves[x+y*BOARD_SIZE][1][1] > 0) {
                for (int i=0; i<BOARD_SIZE*BOARD_SIZE; i++) { // 检测目前谁可以悔棋
					boardData_b[i] = boardData[i];
				}
                nxtMove_b = nxtMove; // 检测目前换谁提供悔棋
                emit undoAvai(true); // 可以悔棋
				drop(x, y);
				printf("User drop: (%d, %d)\n", x, y);
			}
		}

		void drop(int x, int y) {
			if (moves[x+y*BOARD_SIZE][1][1] > 0) {
				doDrop(x, y, nxtMove);
				for (int i=-1; i<2; i++) {
                    for (int j=-1; j<2; j++) { // 遍历九宫格
                        if (!(i == 0 && j == 0)) { // 跳过中间
							for (int d=1; d<=moves[x+y*BOARD_SIZE][1+i][1+j]; d++) {
								doDrop(x+i*d, y+j*d, nxtMove);
							}
						}
					}
				}
				updateScore();
				next();
			} else {
				printf("Illegal move\n");
			}
		}

        /* 换人 */
		void next() {
			if (nxtMove == black) {
				nxtMove = white;
			} else {
				nxtMove = black;
			}
			updateNxtMove();
			aiDrop();
			if (!totalMoves) {
				if (nxtMove == black) {
					nxtMove = white;
				} else {
					nxtMove = black;
				}
				updateNxtMove();
				aiDrop();
				if (!totalMoves) {
					end();
				}
			}
		}

		/* AI 落棋 */
		void aiDrop() {
			if (nxtMove == white && !isAI_W) return;
			if (nxtMove == black && !isAI_B) return;

			int x = -1;
			int y = -1;
			int max = 0;
			for (int i=0; i<BOARD_SIZE; i++) {
				for (int j=0; j<BOARD_SIZE; j++) {
					if (moves[i+j*BOARD_SIZE][1][1] > max ||
							(moves[i+j*BOARD_SIZE][1][1] == max && rand()%2 == 0)) {
						max = moves[i+j*BOARD_SIZE][1][1];
						x = i;
						y = j;
					}
				}
			}
			if (max) {
				drop(x, y);
				printf("AI drop at (%d, %d) because max[%d]\n", x, y, max);
			}
		}

        /* 执行下子, 同时处理内部资料和 ui */
		void doDrop(int x, int y, piece p) {
			printf("doDrop: (%d, %d)\n", x, y);
			boardData[x+y*BOARD_SIZE] = p;
			switch (p) {
				case null:
					guiBoard[x+y*BOARD_SIZE]->setPixmap(*guiImgStoneN);
					break;
				case white:
					guiBoard[x+y*BOARD_SIZE]->setPixmap(*guiImgStoneW);
					break;
				case black:
					guiBoard[x+y*BOARD_SIZE]->setPixmap(*guiImgStoneB);
					break;
			}
		}

		void end() {
			guiTurn->setPixmap(*guiImgStoneN);
			printf("End\n");
		}

        /* 重绘棋盘 */
		void drawBoard() {
			for (int x=0; x<BOARD_SIZE; x++) {
				for (int y=0; y<BOARD_SIZE; y++) {
					int p = getBoardData(x, y);
					switch (p) {
						case null:
							guiBoard[x+y*BOARD_SIZE]->setPixmap(*guiImgStoneN);
							break;
						case white:
							guiBoard[x+y*BOARD_SIZE]->setPixmap(*guiImgStoneW);
							break;
						case black:
							guiBoard[x+y*BOARD_SIZE]->setPixmap(*guiImgStoneB);
							break;
					}
				}
			}
		}

        /* 计算落棋表 */
		void calcMoves(piece who) {
			totalMoves = 0;
            /* 清空表格，-1表示未计算 */
			for (int i=0; i<BOARD_SIZE*BOARD_SIZE; i++) {
				moves[i][1][1] = -1;
			}
			printf("Avail. moves for ");
			(who == black)? printf("black: ") : printf("white: ");
			for (int x=0; x<BOARD_SIZE; x++) {
                for (int y=0; y<BOARD_SIZE; y++) { // 访问每一格
                    if (getBoardData(x, y) != null && getBoardData(x, y) != who) {  // 若此格的棋色和给定的不同
                        calcMoveAround(x, y, who); // 计算周围是否可以落棋
					}
				}
			}
			printf(" [total:%d]\n", totalMoves);
		}

		void calcMoveAround(int x, int y, piece who) {
			for (int i=-1; i<2; i++) {
                for (int j=-1; j<2; j++) { // 九宫格遍历
                    if (getBoardData((x+i), (y+j)) == null && moves[(x+i)+(y+j)*BOARD_SIZE][1][1] == -1) { // 若此格为空, 且未计算过
						calcMoveAt((x+i), (y+j), who); // 算他
					}
				}
			}
		}

		void calcMoveAt(int x, int y, piece who) {
			if (x > BOARD_SIZE-1 || y > BOARD_SIZE-1 || x < 0 || y < 0) return;
			int d;
			piece p;
			moves[x+y*BOARD_SIZE][1][1] = 0;
			for (int i=-1; i<2; i++) {
                for (int j=-1; j<2; j++) { // 九宫格
                    if (!(i == 0 && j == 0)) { // 跳过中间有棋的那一格
                        moves[x+y*BOARD_SIZE][1+i][1+j] = 0; // 重设计数
                        d = 0; // 计数器
                        p = getBoardData(x+i*(d+1), y+j*(d+1)); // 考虑下一颗棋子
                        while (p != null) { // 往下寻找相同颜色
							if (p == who) { // 找到同色
                                moves[x+y*BOARD_SIZE][1+i][1+j] = d; // 写入计数
								moves[x+y*BOARD_SIZE][1][1] += d;
								break;
							}
							d++;
							p = getBoardData(x+i*(d+1), y+j*(d+1));
						}
					}
				}
			}
			if (moves[x+y*BOARD_SIZE][1][1] > 0) {
				totalMoves++;
				printf("(%d, %d) ", x, y);
			}
		}

		void updateNxtMove() {
			if (nxtMove == white) {
				guiTurn->setPixmap(*guiImgStoneW);
				printf("Next: white\n");
			} else {
				guiTurn->setPixmap(*guiImgStoneB);
				printf("Next: black\n");
			}
			calcMoves(nxtMove);
		}

		void updateScore() {
			scoreW = 0;
			scoreB = 0;
			for (int i=0; i<BOARD_SIZE*BOARD_SIZE; i++) {
				if (boardData[i] == white) scoreW++;
				else if (boardData[i] == black) scoreB++;
			}
            if (scoreB + scoreW == BOARD_SIZE*BOARD_SIZE && scoreB<scoreW){
                QMessageBox::about(NULL,"恭喜","红方胜");
            }
            if (scoreB + scoreW == BOARD_SIZE*BOARD_SIZE && scoreB>scoreW){
                QMessageBox::about(NULL,"恭喜","蓝方胜");
            }
			emit updateWScore(scoreW);
			emit updateBScore(scoreB);
		}

        /* 显示提示 */
		void doHint(int x, int y) {
			drawBoard();

			QPixmap *aPix, *sPix;

			if (nxtMove == white) {
				aPix = guiImgStoneWa;
				sPix = guiImgStoneWs;
			} else {
				aPix = guiImgStoneBa;
				sPix = guiImgStoneBs;
			}

			if (x < BOARD_SIZE && y < BOARD_SIZE && x >= 0 && y >= 0) {
				if (moves[x+y*BOARD_SIZE][1][1] > 0) { // 可以下
					guiBoard[x+y*BOARD_SIZE]->setPixmap(*aPix);
					for (int i=-1; i<2; i++) {
                        for (int j=-1; j<2; j++) { // 遍历九宫格
                            if (!(i == 0 && j == 0)) { // 跳过中间
								for (int d=1; d<=moves[x+y*BOARD_SIZE][1+i][1+j]; d++) {
									guiBoard[(x+i*d)+(y+j*d)*BOARD_SIZE]->setPixmap(*sPix);
								}
							}
						}
					}
				}
			}

		}

        bool hint; // 是否显示提示

	public slots:
		void reset() {
			/* Clear the board */
			for (int x=0; x<BOARD_SIZE; x++) {
				for (int y=0; y<BOARD_SIZE; y++) {
					doDrop(x, y, null);
				}
			}
			emit undoAvai(false);

			/* Init */
			doDrop(BOARD_SIZE/2, BOARD_SIZE/2, white);
			doDrop(BOARD_SIZE/2-1, BOARD_SIZE/2-1, white);
			doDrop(BOARD_SIZE/2-1, BOARD_SIZE/2, black);
			doDrop(BOARD_SIZE/2, BOARD_SIZE/2-1, black);
			scoreW = 2;
			scoreB = 2;
			this->nxtMove = white;
			guiTurn->setPixmap(*guiImgStoneW);
			calcMoves(nxtMove);

			updateScore();

			aiDrop();

			printf("Restarted!\n");
			return;
		}

		void hintSwitch(int t) {
			if (t) hint = true;
			else hint = false;
			drawBoard();
			printf("hintSwitch: %d\n", t);
			return;
		}

		void aiWSwitch(int t) {
			if (t) isAI_W = true;
			else isAI_W = false;
			if (isAI_W && nxtMove == white) aiDrop();
			printf("aiWSwitch: %d\n", t);
			return;
		}

		void aiBSwitch(int t) {
			if (t) isAI_B = true;
			else isAI_B = false;
			if (isAI_B && nxtMove == black) aiDrop();
			printf("aiBSwitch: %d\n", t);
			return;
		}

		void undo() {
			printf("Undo\n");
			emit undoAvai(false);
			for (int i=0; i<BOARD_SIZE*BOARD_SIZE; i++) {
				boardData[i] = boardData_b[i];
			}
			nxtMove = nxtMove_b;
			updateNxtMove();
			drawBoard();
			updateScore();
			return;
		}

		void update();

	signals:
        void updateWScore(int); // 红分数更新
        void updateBScore(int); // 蓝分数更新
		void undoAvai(bool);

	private:
        piece boardData[BOARD_SIZE*BOARD_SIZE]; // 棋盘资料
        piece boardData_b[BOARD_SIZE*BOARD_SIZE]; // 备份
        piece nxtMove; // 轮到谁
		piece nxtMove_b;
		int totalMoves;
        int moves[BOARD_SIZE*BOARD_SIZE][3][3]; // 落棋表, 每个位置都是一个二位数组:
        QLabel **guiBoard; // 棋盘 ui              +--------------+--------------+--------------+
        QPixmap *guiImgStoneW; // 红棋子图         |往左上可吃几颗| 往上可吃几颗 |往右上可吃几颗|
        QPixmap *guiImgStoneB; // 蓝棋子图        +--------------+--------------+--------------+
        QPixmap *guiImgStoneWs; // 红棋子小图      | 往左可吃几颗 | 總共可吃几颗 | 往右可吃几颗 |
        QPixmap *guiImgStoneBs; // 蓝棋子小图      +--------------+--------------+--------------+
        QPixmap *guiImgStoneWa; // 红棋子半透明图  |往左下可吃几颗| 往下可吃几颗 |往右下可吃几颗|
        QPixmap *guiImgStoneBa; // 蓝棋子半透明图  +--------------+--------------+--------------+
        QPixmap *guiImgStoneN; // 沒棋子图
        QLabel *guiTurn; // 轮到谁
        int scoreW; // 分数
        int scoreB; // 分数
		bool isAI_W;
		bool isAI_B;
};

#endif
