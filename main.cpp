#include <SDL.h>
#include <SDL_image.h>

#include <iostream>
#include<vector>
#include<string>
#include<stdlib.h>
#include<fstream>
#include <random>
#include <ctime>
#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 600

using namespace std;

SDL_Window* gWindow = NULL; // cửa sổ game
SDL_Renderer* gRenderer = NULL; // màn hình xử lý
Uint32 gOldTime; // tho
float deltaTime = 0; // chênh lệch 2 frame


// struct tọa độ 1 điểm
class Vector2D
{
public:
	float x;
	float y;

	Vector2D()
	{
		x = 0.0f;
		y = 0.0f;
	}

	Vector2D(float a, float b)
	{
		x = a;
		y = b;
	}
	Vector2D operator+(Vector2D other) {
		return Vector2D(x + other.x, y + other.y);
	}
	Vector2D operator-(Vector2D other) {
		return Vector2D(x - other.x, y - other.y);
	}
	Vector2D operator*(float t) {
		return Vector2D(x*t, y*t);
	}
	Vector2D& operator+=(const Vector2D& other) {
		x += other.x;
		y += other.y;
		return *this;
	}
};

// khởi tạo sdl
bool InitSDL()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);


	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		cout << "SDL did not initialise. Error: " << SDL_GetError();
		return false;
	}
	else
	{
		gWindow = SDL_CreateWindow("Games Engine Creation",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			SCREEN_WIDTH,
			SCREEN_HEIGHT,
			SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			cout << "Window was not created. Error : " << SDL_GetError();
			return false;
		}
	}

	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
	if (gRenderer != NULL)
	{
		int imageFlags = IMG_INIT_PNG;
		if (!(IMG_Init(imageFlags) & imageFlags))
		{
			cout << "SDL_Image could not initialise. Error: " << IMG_GetError();
			return false;
		}

	}

	else
	{
		cout << "Renderer could not initialise. Error: " << SDL_GetError();
		return false;
	}

	return true;


}
// đóng sdl
void CloseSDL()
{
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	IMG_Quit();
	SDL_Quit();

	SDL_DestroyRenderer(gRenderer);
	gRenderer = NULL;


}
// load ảnh
SDL_Texture* LoadTextureFromFile(string path)
{
	SDL_Texture* pTexture = NULL;

	SDL_Surface* pSurface = IMG_Load(path.c_str());
	if (pSurface != NULL)
	{
		pTexture = SDL_CreateTextureFromSurface(gRenderer, pSurface);
		if (pTexture == NULL)
		{
			cout << "Unable to create texture from surface. Error: " << SDL_GetError() << endl;
		}
		SDL_FreeSurface(pSurface);
	}
	else
	{
		cout << "Unable to create texture from surface. Error: " << IMG_GetError() << endl;
	}
	return pTexture;
}
void Play(); // chơi
void Menu(); // meni
// class lưu trữ khi load ảnh lên
class Texture2D
{
public:
	// size
	float mWidth;
	float mHeight;

	SDL_Renderer* mRenderer; // màn hình hienr thị
	Vector2D mPosition; // vị trí
	SDL_Texture* mTexture; // biến lưu thông tin
	Texture2D() {
		mRenderer = NULL;
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
	Texture2D(SDL_Renderer* renderer) { // khởi tạo
		mRenderer = renderer;
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
	void Free() { // xóa bộ nhớ
		if (mTexture != NULL)
		{
			SDL_DestroyTexture(mTexture);
			mTexture = NULL;
		}
		mWidth = 0;
		mHeight = 0;
	}
	~Texture2D() { // giải phóng bộ nhớ
		Free();

		mRenderer = NULL;
	}
	bool LoadFromFile(string path) { // load ảnh
		Free(); // giải phóng cũ
		SDL_Surface* pSurface = IMG_Load(path.c_str());

		if (pSurface != NULL)
		{
			SDL_SetColorKey(pSurface, SDL_TRUE, SDL_MapRGB(pSurface->format, 0, 0xFF, 0xFF)); // xóa nền

			mWidth = pSurface->w; // giảm kích thước ban đầu
			mHeight = pSurface->h;

			mTexture = SDL_CreateTextureFromSurface(mRenderer, pSurface);
			if (mTexture == NULL)
			{
				cout << "Unable to create texture from surface. Error: " << SDL_GetError() << endl;
			}
			SDL_FreeSurface(pSurface);
		}
		else
		{
			cout << "Unable to create texture from surface. Error: " << IMG_GetError() << endl;
		}
		return mTexture != NULL;
	}
	// hiển thị đối tượng: vị trí, cách lấy đối xứng, góc xoay, phạm vi lấy, khung hiển thị
	void Render(Vector2D p = { -1, -1 }, SDL_RendererFlip flip = SDL_FLIP_NONE, double angle = 0.0f, SDL_Rect* srcRect = nullptr, SDL_Rect* renderLocation = nullptr) { // load ảnh lên màn hình xử lý
		SDL_SetRenderDrawColor(mRenderer, 0x00, 0x00, 0x00, 0x00);
		// vẽ đối tượng lên màn hình xử lý, với khung vừa có, góc xoay angle, và cách lấy đối xứng flip
		if (p.x == -1)
			p = mPosition;
		if (renderLocation == nullptr){
			SDL_Rect r = { p.x, p.y, mWidth, mHeight };
			SDL_RenderCopyEx(mRenderer, mTexture, srcRect, &r, angle, NULL, flip);
		}
		else {
			SDL_RenderCopyEx(mRenderer, mTexture, srcRect, renderLocation, angle, NULL, flip);
		}
	}
};
 class Math {
public:
	static float Clamp(float value, float min, float max) {
		if (value < min)
			return min;
		if (value > max)
			return max;
		return value;
	}
};
class Slider {
public:
	float minValue, value, maxValue;
	float deltaDelay = 0.5f;
	float _deltaDelay = 0.5f;
	static int dau;
	Texture2D* bg;

	Vector2D mPosition;
	Vector2D mScale;

	Slider(SDL_Renderer* renderer, Vector2D index) {
		bg = new Texture2D(renderer);
		bg->LoadFromFile("./Images/UI/Slider.png");
		mPosition = index;
		minValue = 0;
		maxValue = 17; // vì có 17 frame ảnh
		value = 0;
		dau = 0;
	}
	void Update(float deltaTime = 0) {
		if (_deltaDelay <= 0) { // tạo độ delay để tăng/giảm giá trị
			_deltaDelay = deltaDelay;
			value = Math::Clamp(value + dau, minValue, maxValue);
			if (value == 0 && dau == -1)
				dau = 0;
		}
		else {
			_deltaDelay -= deltaTime;
		}
		Show();
	}
	void Show() { // hiển thị ảnh vì ảnh w = 16, h = độ dài ảnh/17 frame
		SDL_Rect t = { 0, 16*value, 64, bg->mHeight/17 };
		SDL_Rect t2 = { mPosition.x, mPosition.y, 500, 50 }; // vị trí hiển thị
		bg->Render(mPosition, SDL_FLIP_NONE, 0, &t, &t2);
	}

};
int Slider::dau = 0; // giá trị tăng giảm

//static class Audio {
//public:
//	static void Play(int value) {
//		string path = "./Sounds/";
//		string name[] = { "gun.wav", "bom.wav" };
//		Mix_Chunk* sound2 = Mix_LoadWAV((path + name[value]).c_str());
//		Mix_PlayChannel(-1, sound2, 0);
//	}
//};

class Character {
public:
	Texture2D *texture;
	string link; // đường dẫn file tổng
	int frame; // frame đang hiển thị
	float tiLe = 0.02f;
	bool isActive = true;
	bool flipX = false;
	Character(SDL_Renderer* renderer, string link) { // khởi tạo
		texture = new Texture2D(renderer);
		this->link = link;
		frame = 0;
	}
	~Character() {
		texture->Free();
		delete texture;
	}

	virtual void Update(SDL_Event e, float deltaTime = 0){}
};
//class Effect : public Character {
//public:
//	float timeDelay= 0.2f;
//	int frame = 0;
//	float k;
//	Vector2D p;
//	Effect(SDL_Renderer* renderer, Vector2D p, float k = 1) : Character(renderer, "") {
//		this->p = p;
//		this->k = k;
//		texture->LoadFromFile("./Images/Effect/" + to_string(frame) + ".png");
//		texture->mHeight *= k;
//		texture->mWidth *= k;
//		texture->mPosition = p - Vector2D(texture->mWidth / 2, texture->mHeight / 2);
//	}
//	void Update(SDL_Event e, float deltaTime = 0) override {
//		if (frame > 4) return;
//		if (timeDelay > 0) {
//			timeDelay -= deltaTime;
//		}
//		else {
//			timeDelay = 0.2f;
//			frame++;
//			if (frame > 8) {
//				isActive = false;
//			}
//			else {
//				//cout << "./Images/Effect/" + to_string(frame) + ".png" << endl;
//				texture->LoadFromFile("./Images/Effect/" + to_string(frame) + ".png");
//				texture->mHeight *= k;
//				texture->mWidth *= k;
//				texture->mPosition = p - Vector2D(texture->mWidth / 2, texture->mHeight / 2);
//			}
//		}
//		texture->Render(Vector2D(-1, -1), flipX ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
//	}
//};

class Player : public Character {
public:
	Vector2D move;
	vector<Texture2D*> textureStates; // danh sách các biến lưu trạng thái lần lượt: idle, run, attack, die
	Vector2D mPosition; // vị trí
	Vector2D mScale; // size
	int state; // 0: idle, 1: run, 2: attack, 3: die
	float next_frame = 0.2f;
	float _next_frame = 0.2f;

	Player(SDL_Renderer* renderer, Vector2D p) : Character(renderer, "") {
		string path = "./Images/Thief/";
		string name[] = { "Idle", "Run", "Attack", "Death" };
		string duoi = ".png";
		state = 0;
		for (int i = 0; i < 4; i++) { // load các biến lưu trạng thái
			Texture2D* t = new Texture2D(renderer);
			t->LoadFromFile(path + name[i] + duoi);
			/*t->mHeight = 100;
			t->mWidth = 50;*/
			//cout << t->mWidth << " " << t->mHeight << endl;
			textureStates.push_back(t);
		}
		this->mPosition = p; // lưu vị trí, size
		this->mScale = Vector2D(textureStates[0]->mWidth*0.8f, textureStates[0]->mHeight* 1.2f);
	}
	~Player() {
		texture->Free();
	}
	void CheckInput(SDL_Event &e) {
		switch (e.type)
		{
			case SDL_KEYDOWN: // bấm phím
				switch (e.key.keysym.sym)
				{
				case SDLK_RETURN: {
					if (state == 1 || state == 2) break; // nếu đang chạy hoặc tấn công thì bỏ qua
					state = 1; // set trạng thái chạy
					Slider::dau = 1; // tăng giá trị slider
					frame = 0; // cài lại frame
					break;
				}
				}
				break;
			case SDL_KEYUP: // bấm phím
				switch (e.key.keysym.sym)
				{
				case SDLK_RETURN: {
					state = 0; // trạng thái đứng
					Slider::dau = -1; // giảm gtri slider
					frame = 0;
					break;
				}
				}
		}
	}
	void Attack() { // chuyển đổi sang trạng thái tấn công
		if (state == 2) return;
		state = 2;
		frame = 0;
	}
	void Death() { // chuyển đổi sang trạng thái die
		if (state == 3) return;
		state = 3;
		frame = 0;
	}

	void Update(SDL_Event e, float deltaTime = 0) override { // cập nhật
		CheckInput(e); // check sự kiện
		UpdateAnimation(deltaTime); // cập nhật hình ảnh
	}
	void UpdateAnimation(float deltaTime) {
		int frameMax[] = { 4, 8, 4, 7 };

		if (_next_frame <= 0) {
			_next_frame = next_frame*(state == 3 ? 2 : 1); // khi tấn công, tốc độ frame sẽ giảm gấp 2 lần bình thường
			if (frame >= frameMax[state] - 1) { // hết hoạt ảnh
				frame = 0; // reset
				switch (state)
				{
				case 2: {
					state = 0; // tấn công xong sẽ trở về đứng
					break;
				}
				case 3: {
					frame = 6; // giữ nguyên trạng thái die
					break;
				}
				}
			}
			else {
				frame++;// tăng giá trị frame
			}
		}
		else {
			_next_frame -= deltaTime; // giảm thời gian đợi hoạt ảnh
		}

		SDL_Rect t = { 200*frame, 0, 200, textureStates[state]->mHeight }; // kích thước mỗi frame là 200*200
		SDL_Rect t2 = { mPosition.x, mPosition.y, mScale.x, mScale.y };
		textureStates[state]->Render(mPosition, SDL_FLIP_NONE, 0, &t, &t2); // hiển thị
	}
};
class King : public Character {
public:
	Vector2D move;
	vector<Texture2D*> textureStates;
	Vector2D mPosition;
	Vector2D mScale;
	int state; // 0: idle, 1: run, 2: attack, 3: die
	float next_frame = 0.2f;
	float _next_frame = 0.2f;

	float deltaRandom = 0;
	bool flip;

	King(SDL_Renderer* renderer, Vector2D p) : Character(renderer, "") {
		string path = "./Images/King/";
		string name[] = { "Idle", "Run", "Attack", "Death" };
		string duoi = ".png";
		state = 0;
		flip = true;
		for (int i = 0; i < 4; i++) {
			Texture2D* t = new Texture2D(renderer);
			t->LoadFromFile(path + name[i] + duoi);
			textureStates.push_back(t);
		}
		this->mPosition = p;
		this->mScale = Vector2D(textureStates[0]->mWidth * 0.5f, textureStates[0]->mHeight * 0.8f);
	}
	~King() {
		texture->Free();
	}
	void Attack() {
		if (state == 2) return;
		state = 2;
		frame = 0;
		flip = true;
	}
	void Death() {
		if (state == 3) return;
		state = 3;
		frame = 0;
	}

	void Update(SDL_Event e, float deltaTime = 0) override {
		RandomState(deltaTime);
		UpdateAnimation(deltaTime);
	}
	void RandomState(float deltaTime = 0) {
		if (state == 3) return;// nếu đang tấn công thì kết thúc
		deltaRandom -= deltaTime; // giảm thời gian tạo biến ngẫu nhiên
		if (deltaRandom <= 0) {// nếu thời gian chạy <= 0
			if (deltaRandom <= -1.0f) { // nếu thời gian quay mặt <= -1
				std::mt19937_64 rng(std::time(nullptr)); // Khởi tạo generator ngẫu nhiên với seed từ thời gian hiện tại
				std::uniform_real_distribution<double> dist(0.0, 5.0); // Phạm vi từ 0 đến 5
				deltaRandom = dist(rng);
				state = 1; // run
				frame = 0;
				flip = false; // run thì sẽ k lật ảnh theo trục Y
			}
			else {
				if (state != 0) {// trạng thái khác đứng
					flip = true;  // lật ảnh
					state = 0; // trạng thái đứng
					frame = 0;
				}
			}
		}
	}
	void UpdateAnimation(float deltaTime) {
		int frameMax[] = { 6, 8, 6, 11};

		if (_next_frame <= 0) {
			_next_frame = next_frame;
			if (frame >= frameMax[state] - 1) {
				frame = 0;
				switch (state)
				{
				case 2: {
					state = 0;
					break;
				}
				case 3: {
					frame = 10;
					break;
				}
				}
			}
			else {
				frame++;
			}
		}
		else {
			_next_frame -= deltaTime;
		}

		SDL_Rect t = { 155 * frame, 0, 155, textureStates[state]->mHeight }; // size 155*155
		SDL_Rect t2 = { mPosition.x, mPosition.y, mScale.x, mScale.y };
		textureStates[state]->Render(mPosition, !flip ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL, 0, &t, &t2);
	}
};
//class Map {
//public:
//	int** matrix;
//	int m, n;
//	vector<Texture2D*> values;
//	SDL_Renderer* renderer;
//	Map(SDL_Renderer* renderer) {
//		this->renderer = renderer;
//		string part = "./Images/Map/";
//		string duoi = ".png";
//		string names[] = { "0", "1", "2", "3" };
//		Vector2D size = { 50, 50 };
//		for (int i = 0; i < 4; i++) {
//			Texture2D* t = new Texture2D(renderer);
//			t->mHeight = size.y;
//			t->mWidth = size.x;
//			//cout << t->mWidth << " " << t->mHeight << endl;
//			values.push_back(t);
//		}
//	}
//	void LoadMap(int** a, int m, int n) {
//		matrix = a;
//		this->m = m;
//		this->n = n;
//	}
//	~Map() {
//		for (int i = 0; i < m; i++)
//			delete[] matrix[i];
//		delete matrix;
//	}
//	void Update() {
//
//		Show();
//	}
//	void Show() {
//		for (int i = 0; i < m; i++) {
//			for (int j = 0; j < n; j++) {
//				if (matrix[i][j] != -1) {
//					values[matrix[i][j]]->Render(Vector2D(50 * j, 50 * (n - i)));
//				}
//			}
//		}
//	}
//
//};
class BackGround {
public:
	vector<Character*> BG; // các biến lưu ảnh
	float speed = 0.01f; // tốc độ chạy của ảnh
	bool isRunning = false; // ảnh chạy
	BackGround(SDL_Renderer* renderer, int type = 0) {
		string part = "./Images/Map/";
		string duoi = ".png";
		string names[] = { "BG1", "BG2"};
		for (int i = 0; i < 2; i++) {
			Character* bg = new Character(renderer, "");
			bg->texture->LoadFromFile(part + names[i] + duoi);
			bg->texture->mHeight = SCREEN_HEIGHT;
			bg->texture->mWidth = SCREEN_WIDTH;

			BG.push_back(bg);
		}
		if (type == 0) {
			Character* t = new Character(renderer, "");
			t->texture->LoadFromFile(part + "Salt" + duoi);
			t->texture->mHeight /= 0.75f;
			t->texture->mWidth /= 0.75f;
			//cout << t->texture->mHeight << " " << t->texture->mWidth;
			t->texture->mPosition = Vector2D(SCREEN_WIDTH / 2 - t->texture->mWidth / 2, SCREEN_HEIGHT - t->texture->mHeight);
			BG.push_back(t);
		}
		else {
			Character* t = new Character(renderer, "");
			t->texture->LoadFromFile(part + "BG3" + duoi);
			t->texture->mHeight /= 0.75f;
			t->texture->mWidth /= 0.5f;
			//cout << t->texture->mHeight << " " << t->texture->mWidth;
			t->texture->mPosition = Vector2D(SCREEN_WIDTH / 2 - t->texture->mWidth / 2, SCREEN_HEIGHT - t->texture->mHeight);
			BG.push_back(t);

			Character* t2 = new Character(renderer, "");
			t2->texture->LoadFromFile(part + "BG3" + duoi);
			t2->texture->mHeight /= 0.75f;
			t2->texture->mWidth /= 0.5f;
			//cout << t->texture->mHeight << " " << t->texture->mWidth;
			t2->texture->mPosition = Vector2D(SCREEN_WIDTH / 2 + t2->texture->mWidth / 2, SCREEN_HEIGHT - t2->texture->mHeight);
			BG.push_back(t2);
		}
	}
	void Update() {
		for (int i = 0; i < BG.size(); i++) {
			BG[i]->texture->Render(Vector2D(-1, -1), SDL_FLIP_NONE); // hiển thị ảnh
		}
		if(isRunning)
			Move();
	}
	void Move() {
		Move(2);
		Move(3);
	}
	void Move(int index) { // giảm vị trí x của đối tượng index
		if (index < BG.size()) {
			BG[index]->texture->mPosition.x -= speed;
			if (BG[index]->texture->mPosition.x <= -SCREEN_WIDTH) // tạo vòng lặp
				BG[index]->texture->mPosition.x = SCREEN_WIDTH;
		}
	}
	void Free() {
		for (int i = 0; i < BG.size(); i++){
			BG[i]->texture->Free();
		}
		BG.clear();
	}
};
class Button {
public:
	Character* img;
	Vector2D size;
	Vector2D position;
	int type;
	bool isActive;
	Button(SDL_Renderer* renderer, int type, Vector2D p, float k) {
		img = new Character(renderer, "");
		this->type = type;
		this->isActive = true;
		string path = "./Images/UI/";
		string linksTxt[] = { "txtPlay.png", "txtQuit.png" };
		img->texture->LoadFromFile(path + linksTxt[type]);
		img->texture->mHeight *= k;
		img->texture->mWidth *= k;
		size = Vector2D(img->texture->mWidth, img->texture->mHeight);
		position = p;
		img->texture->mPosition = p - Vector2D(img->texture->mWidth, img->texture->mHeight)*0.5f;
	}
	~Button() {
		delete img;
	}
	void OnMouse(SDL_Event e) { // kiểm tra sự kiện
		int x = e.button.x;
		int y = e.button.y;
		if (x >= img->texture->mPosition.x && x <= img->texture->mPosition.x + img->texture->mWidth
			&& y >= img->texture->mPosition.y && y <= img->texture->mPosition.y + img->texture->mHeight) { // chuột ngang qua
			OnHigh();  // hiệu ứng phóng to
			switch (e.type)
			{
			case SDL_MOUSEBUTTONUP: {
				OnClick(); // gọi sk click
				break;
			}
			}

		}
		else {
			// trả về size ban đầu
			img->texture->mWidth = size.x;
			img->texture->mHeight = size.y;
			img->texture->mPosition = position - Vector2D(img->texture->mWidth, img->texture->mHeight) * 0.5f;
		}

	}
	void OnClick() {
		isActive = false;
	}
	void OnHigh() {
		float value = 1;
		img->texture->mWidth = Math::Clamp(img->texture->mWidth + 10, 0, size.x*1.2f); // Tăng chiều rộng lên 10 pixel
		img->texture->mHeight = Math::Clamp(img->texture->mHeight + 10, 0, size.y * 1.2f); // Tăng chiều cao lên 10 pixel
		img->texture->mPosition = position - Vector2D(img->texture->mWidth, img->texture->mHeight) * 0.5f;
	}
	void Update(SDL_Event e) {
		OnMouse(e);
		Render();
	}
	void Render() {
		img->texture->Render(Vector2D(-1, -1));
	}
};

int main(int argc, char* args[])
{
	if (InitSDL())
	{
		srand(time(0)); // cập nhật thời gian hiện tại để làm mới random


		Menu();
		//Play();
	}
	CloseSDL();
	return 0;
}

void Play() {
	SDL_Event e;
	BackGround bg(gRenderer, -1); // tạo bg
	//{
		//int m, n;
		//int** a = LoadMap(m, n);
		///*for (int i = 0; i < m; i++) {
		//	for (int j = 0; j < n; j++)
		//		cout << a[i][j] << " ";
		//	cout << endl;
		//}*/
		//Map map(gRenderer);
		//map.LoadMap(a, m, n);
	//}
	Button quit(gRenderer, 1, Vector2D(850, 40), 0.15f); // tạo nút thoát

	Player *player = new Player(gRenderer, Vector2D(-150, 385)); // tạo ng chơi
	King* king = new King(gRenderer, Vector2D(100, 440)); // tạo king
	Slider* slider = new Slider(gRenderer, Vector2D(200, 50)); // tạo slider
	bool endGame = false;
	float deltatime = 0; // chenh lẹch 2 frame
	while (!endGame) {
		gOldTime = SDL_GetTicks(); // lấy thời gian hiện tại
		SDL_PollEvent(&e); // sự kiện
		SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0x00);
		SDL_RenderClear(gRenderer); // xóa dữ liệu cũ

		bg.Update(); // cập nhật back ground


		if (!quit.isActive)
			endGame = true;

		king->Update(e, deltatime); // update king
		if (king->state != 2 && player->state != 3 && player->frame != 6) // nếu king k tấn công và người chơi không chết thì update người chơi
			player->Update(e, deltatime);

		if (player->state == 2 && player->frame >= 1 && king->state == 0) { // nếu ng chơi tấn công tại frame 1 và king quay lại
			king->Attack(); // king tấn công
			player->Death(); // player die
			slider->value = 0; // reset value
		}
		if ((king->state == 2 && king->frame == 11) || (king->state == 3 && king->frame == 10)) break; // king tấn công xong và kết thúc hoặc king die
		if (king->state != 2) { // king không taasn công
			slider->Update(deltatime); // cập nhật slider
			if (slider->value == slider->maxValue) { // nếu đầy thì tấn công
				player->Attack();
				if (player->frame == 3) // nếu đến frame 3 thì king die
					king->Death();
			}
		}
		quit.Update(e);
		SDL_zero(e);
		SDL_RenderPresent(gRenderer); // hiển thị ra màn hình
		deltatime = (float)(SDL_GetTicks() - gOldTime) / 1000.0f; // tính độ chênh giữa 2
	}
	bg.Free();
}
void Menu() {
	SDL_Event e;
	BackGround bg(gRenderer);

	Button play(gRenderer, 0, Vector2D(250, 400), 0.2f);
	Button quit(gRenderer, 1, Vector2D(650, 400), 0.2f);
	float deltatime = 0; // chenh lẹch 2 frame
	while (true) {
		SDL_PollEvent(&e); // sự kiện
		gOldTime = SDL_GetTicks(); // lấy thời gian hiện tại
		SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0x00);
		SDL_RenderClear(gRenderer); // xóa dữ liệu cũ

		bg.Update();
		play.Update(e);
		quit.Update(e);

		SDL_RenderPresent(gRenderer); // hiển thị ra màn hình
		if (!play.isActive) { // nếu bấm nút play
			Play();
			play.isActive = true; // reset lại
			e.button.x = -1;
		}
		else
			if (!quit.isActive) {
				return;
			}
		deltatime = (float)(SDL_GetTicks() - gOldTime) / 1000.0f; // tính độ chênh giữa 2
	}
}

class Effect : public Character {
public:
	float timeDelay= 0.2f;
	int frame = 0;
	float k;
	Vector2D p;
	Effect(SDL_Renderer* renderer, Vector2D p, float k = 1) : Character(renderer, "") {
		this->p = p;
		this->k = k;
		texture->LoadFromFile("./Images/Effect/" + to_string(frame) + ".png");
		texture->mHeight *= k;
		texture->mWidth *= k;
		texture->mPosition = p - Vector2D(texture->mWidth / 2, texture->mHeight / 2);
	}
	void Update(SDL_Event e, float deltaTime = 0) override {
		if (frame > 4) return;
		if (timeDelay > 0) {
			timeDelay -= deltaTime;
		}
		else {
		timeDelay = 0.2f;
		frame++;
		if (frame > 8) {
			isActive = false;
			}
			else {
				//cout << "./Images/Effect/" + to_string(frame) + ".png" << endl;
				texture->LoadFromFile("./Images/Effect/" + to_string(frame) + ".png");
			texture->mHeight *= k;
			texture->mWidth *= k;
			texture->mPosition = p - Vector2D(texture->mWidth / 2, texture->mHeight / 2);
		}
		}
		texture->Render(Vector2D(-1, -1), flipX ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
	}
};
