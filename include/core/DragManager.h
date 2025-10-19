#pragma once
#include "HoverManager.h"

class DragManager : public HoverManager
{
private:
	Optional<int> m_heldIndex = none;
	Vec2 m_offset = Vec2{ 0, 0 };
	bool m_isDragging = false;

	// *** 修正点 (4) ***
	// ドロップされた「瞬間」を検知するための変数
	bool m_wasDropped = false;
	Optional<int> m_droppedIndex = none; // ドロップされたカードのインデックス
	Vec2 m_currentPos = Vec2{ 0, 0 };
public:
	DragManager() = default;

	void updateDrag(Array<RectF>& cardRects)
	{
		// フレームの開始時にドロップ状態をリセット
		m_wasDropped = false;
		m_droppedIndex = none;

		m_currentPos = Cursor::PosF();

		// --- 3. ドラッグ中の処理 ---
		if (m_isDragging)
		{
			// マウスボタンが離されたらドロップ処理
			if (MouseL.up())
			{
				m_wasDropped = true;
				m_droppedIndex = m_heldIndex;
				m_isDragging = false;
				m_heldIndex = none;
			}
		}
		// --- 1. ドラッグ中でない時の処理 ---
		else
		{
			updateHover(cardRects);

			// --- 2. ドラッグ開始の判定 ---
			if (MouseL.down())
			{
				// ホバー中のカードがあったら、ドラッグ状態に移行
				if (m_hoveredIndex)
				{
					m_heldIndex = m_hoveredIndex;
					m_offset = m_currentPos - cardRects[*m_heldIndex].center();
					m_isDragging = true;
				}
			}
		}
	}

	// --- ゲッター (Getter) ---
	bool isDragging() const { return m_isDragging; }
	Optional<int> heldIndex() const { return m_heldIndex; }

	// --- ドロップ検知用のゲッター ---
	// マウスが離された「瞬間」だけ true を返す
	bool wasJustDropped() const { return m_wasDropped; }
	// マウスが離された瞬間に、どのカードがドロップされたかを返す
	Optional<int> droppedIndex() const { return m_droppedIndex; }

	void clearHover() { m_hoveredIndex = none; }

	RectF getDraggingRect(double cardWidth, double cardHeight) const

	{

		if (!m_isDragging || !m_heldIndex) return RectF{ 0, 0, 0, 0 };

		return RectF{ Arg::center(m_currentPos - m_offset), cardWidth, cardHeight };

	}

};
