#include "Window.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QMouseEvent>
#include <QLabel>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QVBoxLayout>
#include <QScreen>
#include <QSlider>

#include <array>
#include <cmath>

namespace
{

constexpr std::array<GLfloat, 8u> vertices = {
	-1.0f, -1.0f,
	-1.0f, 1.0f,
	1.0f, -1.0f,
	1.0f, 1.0f
};
constexpr std::array<GLuint, 6u> indices = {
	0, 1, 2,
	1, 2, 3
};

}// namespace

template <typename Func1, typename Func2>
QLayout * Window::createSlider(int min, int max, int defaultValue, Func1 slot, Func2 valueFormat)
{
	auto layout = new QHBoxLayout();
	auto slider = new QSlider(Qt::Horizontal);
	slider->setRange(min, max);
	slider->setValue(defaultValue);
	auto valueLabel = new QLabel(valueFormat(defaultValue));
	layout->addWidget(slider);
	layout->addWidget(valueLabel);

	connect(
		slider, &QSlider::valueChanged, this,
		[=, slot = std::move(slot)](int value) {
			slot(value);
			valueLabel->setText(valueFormat(value));
		}
	);
	return layout;
}

template <typename Func>
QLayout * Window::createIntSlider(int min, int max, int defaultValue, Func slot)
{
	return createSlider(
		min, max, defaultValue, std::move(slot),
		[](int value) { return QString::number(value); }
	);
}

template <typename Func>
QLayout * Window::createFloatSlider(float min, float max, float defaultValue, float step, Func slot)
{
	return createSlider(
		static_cast<int>(min / step), static_cast<int>(max / step),
		static_cast<int>(defaultValue / step),
		[slot = std::move(slot), step](int value) { slot(value * step); },
		[step](int value) { return QString::number(value * step); }
	);
}

Window::Window() noexcept
{
	const auto formatFPS = [](const auto value) {
		return QString("FPS: %1").arg(QString::number(value));
	};

	auto fps = new QLabel(formatFPS(0), this);
	fps->setStyleSheet("QLabel { background-color : black; color : white; }");
	auto fpsLayout = new QHBoxLayout();
	fpsLayout->addWidget(fps);
	fpsLayout->addStretch(1);

	auto fractalSettings = new QGroupBox("Fractal Settings");
	auto fractalSettingsLayout = new QFormLayout();
	fractalSettingsLayout->addRow("Re(c):", createFloatSlider(
		-1.f, 1.f, c_[0], 1e-4,
		[this](float value) { c_[0] = value; }
	));
	fractalSettingsLayout->addRow("Im(c):", createFloatSlider(
		-1.f, 1.f, c_[1], 1e-4,
		[this](float value) { c_[1] = value; }
	));
	fractalSettings->setLayout(fractalSettingsLayout);

	auto qualitySettings = new QGroupBox("Quality Settings");
	auto qualitySettingsLayout = new QFormLayout();
	qualitySettingsLayout->addRow("Iterations:", createIntSlider(
		1, 1024, maxIterations_,
		[this](int value) { maxIterations_ = value; }
	));
	qualitySettingsLayout->addRow("Escape radius:", createFloatSlider(
		0.1f, 10.f, sqrt(radiusThresholdSquared_), 0.1f,
		[this](float value) { radiusThresholdSquared_ = value * value; }
	));
	qualitySettings->setLayout(qualitySettingsLayout);

	auto layout = new QVBoxLayout();
	layout->addLayout(fpsLayout);
	layout->addStretch(1);
	layout->addWidget(fractalSettings);
	layout->addWidget(qualitySettings);

	setLayout(layout);

	timer_.start();

	connect(this, &Window::updateUI, [=, this] {
		fps->setText(formatFPS(ui_.fps));
	});
}

Window::~Window()
{
	{
		// Free resources with context bounded.
		const auto guard = bindContext();
		program_.reset();
	}
}

void Window::onInit()
{
	// Configure shaders
	program_ = std::make_unique<QOpenGLShaderProgram>(this);
	program_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/julia.vs");
	program_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/julia.fs");
	program_->link();

	// Create VAO object
	vao_.create();
	vao_.bind();

	// Create VBO
	vbo_.create();
	vbo_.bind();
	vbo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
	vbo_.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(GLfloat)));

	// Create IBO
	ibo_.create();
	ibo_.bind();
	ibo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
	ibo_.allocate(indices.data(), static_cast<int>(indices.size() * sizeof(GLuint)));

	// Bind attributes
	program_->bind();

	program_->enableAttributeArray(0);
	program_->setAttributeBuffer(0, GL_FLOAT, 0, 2, static_cast<int>(2 * sizeof(GLfloat)));

	cLoc_ = program_->uniformLocation("c");
	radiusThresholdSquaredLoc_ = program_->uniformLocation("radiusThresholdSquared");
	maxIterationsLoc_ = program_->uniformLocation("maxIterations");
	posScaleLoc_ = program_->uniformLocation("posScale");
	centerLoc_ = program_->uniformLocation("center");
	aColorLoc_ = program_->uniformLocation("aColor");
	bColorLoc_ = program_->uniformLocation("bColor");
	cColorLoc_ = program_->uniformLocation("cColor");
	dColorLoc_ = program_->uniformLocation("dColor");

	// Release all
	program_->release();

	vao_.release();

	ibo_.release();
	vbo_.release();

	// Clear all FBO buffers
	glClear(GL_COLOR_BUFFER_BIT);
}

void Window::onRender()
{
	const auto guard = captureMetrics();

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT);

	// Bind VAO and shader program
	program_->bind();
	vao_.bind();

	// Update uniform values
	program_->setUniformValue(cLoc_, c_);
	program_->setUniformValue(radiusThresholdSquaredLoc_, radiusThresholdSquared_);
	program_->setUniformValue(maxIterationsLoc_, maxIterations_);

	program_->setUniformValue(posScaleLoc_, scale_ * (resolution_ / 2));
	program_->setUniformValue(centerLoc_, center_);

	program_->setUniformValue(aColorLoc_, QVector3D(.5f, .5f, .5f));
	program_->setUniformValue(bColorLoc_, QVector3D(.5f, .5f, .5f));
	program_->setUniformValue(cColorLoc_, QVector3D(1.0f, 1.0f, 1.0f));
	program_->setUniformValue(dColorLoc_, QVector3D(.0f, .1f, .2f));

	// Draw
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	// Release VAO and shader program
	vao_.release();
	program_->release();

	++frameCount_;

	// Request redraw if animated
	if (animated_)
	{
		update();
	}
}

void Window::onResize(const size_t width, const size_t height)
{
	// Configure viewport
	glViewport(0, 0, static_cast<GLint>(width), static_cast<GLint>(height));
	scale_ *= std::min(resolution_.x(), resolution_.y()) / std::min(width, height);
	resolution_ = {static_cast<GLfloat>(width), static_cast<GLfloat>(height)};
}

QVector2D Window::screenToField(QVector2D vec) const
{
	vec[1] *= -1;
	return vec * scale_;
}

void Window::mousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
	{
		dragging_ = true;
		lastMousePos_ = event->pos();
	}
}

void Window::mouseMoveEvent(QMouseEvent * event)
{
	if (dragging_)
	{
		center_ -= screenToField(QVector2D(event->pos() - lastMousePos_));
		lastMousePos_ = event->pos();
	}
}

void Window::mouseReleaseEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
	{
		dragging_ = false;
	}
}

void Window::wheelEvent(QWheelEvent * event)
{
	const float zoomStep = 1.1f;
	const float zoom = (event->angleDelta().y() > 0) ? zoomStep : 1 / zoomStep;
	auto delta = screenToField(QVector2D(event->position()) - resolution_ / 2);

	center_ += delta * (1 - 1 / zoom);
	scale_ /= zoom;
}

Window::PerfomanceMetricsGuard::PerfomanceMetricsGuard(std::function<void()> callback)
	: callback_{ std::move(callback) }
{
}

Window::PerfomanceMetricsGuard::~PerfomanceMetricsGuard()
{
	if (callback_)
	{
		callback_();
	}
}

auto Window::captureMetrics() -> PerfomanceMetricsGuard
{
	return PerfomanceMetricsGuard{
		[&] {
			if (timer_.elapsed() >= 1000)
			{
				const auto elapsedSeconds = static_cast<float>(timer_.restart()) / 1000.0f;
				ui_.fps = static_cast<size_t>(std::round(frameCount_ / elapsedSeconds));
				frameCount_ = 0;
				emit updateUI();
			}
		}
	};
}
