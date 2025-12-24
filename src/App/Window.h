#pragma once

#include <Base/GLWidget.hpp>

#include <QElapsedTimer>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include <functional>
#include <memory>

class Window final : public fgl::GLWidget
{
	Q_OBJECT
public:
	Window() noexcept;
	~Window() override;

public: // fgl::GLWidget
	void onInit() override;
	void onRender() override;
	void onResize(size_t width, size_t height) override;

protected:
	void mousePressEvent(QMouseEvent * event) override;
	void mouseMoveEvent(QMouseEvent * event) override;
	void mouseReleaseEvent(QMouseEvent * event) override;
	void wheelEvent(QWheelEvent * event) override;

private:
	QVector2D screenToField(QVector2D vec) const;

	template <typename Func1, typename Func2>
	QLayout * createSlider(int min, int max, int defaultValue, Func1 slot, Func2 valueFormat);
	template <typename Func>
	QLayout * createIntSlider(int min, int max, int defaultValue, Func slot);
	template <typename Func>
	QLayout * createFloatSlider(float min, float max, float defaultValue, float step, Func slot);

private:
	class PerfomanceMetricsGuard final
	{
	public:
		explicit PerfomanceMetricsGuard(std::function<void()> callback);
		~PerfomanceMetricsGuard();

		PerfomanceMetricsGuard(const PerfomanceMetricsGuard &) = delete;
		PerfomanceMetricsGuard(PerfomanceMetricsGuard &&) = delete;

		PerfomanceMetricsGuard & operator=(const PerfomanceMetricsGuard &) = delete;
		PerfomanceMetricsGuard & operator=(PerfomanceMetricsGuard &&) = delete;

	private:
		std::function<void()> callback_;
	};

private:
	[[nodiscard]] PerfomanceMetricsGuard captureMetrics();

signals:
	void updateUI();

private:
	GLint cLoc_ = -1;
	GLint radiusThresholdSquaredLoc_ = -1;
	GLint maxIterationsLoc_ = -1;

	GLint posScaleLoc_ = -1;
	GLint centerLoc_ = -1;

	GLint aColorLoc_ = -1;
	GLint bColorLoc_ = -1;
	GLint cColorLoc_ = -1;
	GLint dColorLoc_ = -1;

	QOpenGLBuffer vbo_{QOpenGLBuffer::Type::VertexBuffer};
	QOpenGLBuffer ibo_{QOpenGLBuffer::Type::IndexBuffer};
	QOpenGLVertexArrayObject vao_;

	QVector2D c_{-0.7269f, 0.1889f};
	GLfloat radiusThresholdSquared_ = 4.f;
	unsigned int maxIterations_ = 512;

	QVector2D resolution_{2, 2};
	QVector2D center_{};
	GLfloat scale_ = 1.f;

	std::unique_ptr<QOpenGLShaderProgram> program_;

	QElapsedTimer timer_;
	size_t frameCount_ = 0;

	struct {
		size_t fps = 0;
	} ui_;

	bool animated_ = true;

	bool dragging_ = false;
	QPoint lastMousePos_;
};
