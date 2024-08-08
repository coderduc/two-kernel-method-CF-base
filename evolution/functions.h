#pragma once
#define PITCH	0
#define YAW		1
#define ROLL	2

//case 1: BONE_ID = 6; break;//Head
//case 2: BONE_ID = 5; break;//Neck
//case 3: BONE_ID = 2; break;//Body
//case 4: BONE_ID = 10; break;//LeftHand
//case 5: BONE_ID = 17; break;//RightHand
//case 6: BONE_ID = 23; break;//FeftFoot
//case 7: BONE_ID = 28; break;//RightFoor

#define MOUSE_MOVE_RELATIVE         0
#define MOUSE_MOVE_ABSOLUTE         1
#define MOUSE_LEFT_BUTTON_DOWN   0x0001  // Left Button changed to down.
#define MOUSE_LEFT_BUTTON_UP     0x0002  // Left Button changed to up.
#define MOUSE_RIGHT_BUTTON_DOWN  0x0004  // Right Button changed to down.
#define MOUSE_RIGHT_BUTTON_UP    0x0008  // Right Button changed to up.
#define MOUSE_MIDDLE_BUTTON_DOWN 0x0010  // Middle Button changed to down.
#define MOUSE_MIDDLE_BUTTON_UP   0x0020  // Middle Button changed to up.

HWND Entryhwnd = NULL, CF = NULL;
ULONG64 CShell_x64 = 0;
int processid = 0;
RECT rect;

//===================================
int iBoneID = 5;
float fSmooth = 1, fFov = 32.0f;
int AimKey = VK_RBUTTON;
int TriggerKey = VK_XBUTTON1;
bool isAimbot = true;
bool isTrigger = false;
bool isNoRecoil = false;
//===================================

int WindowWidth = 1600, WindowHeight = 900;

DWORD64 dwmatrix = 0x1410C6630;
DWORD64 dwLTShell = 0x2BC0030;
DWORD64 dwENT_BEGIN = 0x288;
DWORD64 dwENT_SIZE = 0x2088;
DWORD64 dwLOCAL_ENT = 0x90;
DWORD64 dwLOCAL_ENT_INDEX = 0x282;
DWORD64 ViewAngle = 0x854;
DWORD64 ModelDB = 0x16C8;
DWORD64 CachedTransform = 0x28F8;
DWORD64 dwNoRecoilOffset = 0x12FC;

#define GetPlayerByIndex( LTClientShell, ID ) ( CPlayer * )( LTClientShell + ( ID * dwENT_SIZE ) + dwENT_BEGIN )
uintptr_t LTClientShell;

InterceptionContext context;
InterceptionDevice device;
InterceptionStroke stroke;

inline void NormalMouse() {
	while (interception_receive(context, device = interception_wait(context), &stroke, 1) > 0) {
		if (interception_is_mouse(device))
		{
			InterceptionMouseStroke& mstroke = *(InterceptionMouseStroke*)&stroke;
			interception_send(context, device, &stroke, 1);
		}
	}
}

inline void InitMoveMouse() {
	cout << OBF("[>] Loading...") << endl;

	context = interception_create_context();
	interception_set_filter(context, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_MOVE);
	device = interception_wait(context);

	while (interception_receive(context, device = interception_wait(context), &stroke, 1) > 0) {
		if (interception_is_mouse(device))
		{
			InterceptionMouseStroke& mstroke = *(InterceptionMouseStroke*)&stroke;
			interception_send(context, device, &stroke, 1);
			break;
		}
	}
	system(OBF("cls"));
	cout << OBF("[+] Loaded !") << endl;
	thread normal(NormalMouse);
	normal.detach();
}

inline void mouse_move(int dx, int dy) {
	InterceptionMouseStroke& mstroke = *(InterceptionMouseStroke*)&stroke;
	mstroke.flags = 0;
	mstroke.information = 0;
	mstroke.x = dx;
	mstroke.y = dy;
	interception_send(context, device, &stroke, 1);
}

inline void mouse_click() {
	InterceptionMouseStroke& mstroke = *(InterceptionMouseStroke*)&stroke;
	mstroke.state = INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN;
	interception_send(context, device, &stroke, 1);
	mstroke.state = INTERCEPTION_MOUSE_LEFT_BUTTON_UP;
	interception_send(context, device, &stroke, 1);
	Sleep(5);
}

BITMAPINFOHEADER getBitmapInfoHeader(int& width, int& height) {
	BITMAPINFOHEADER bminfoheader;
	ZeroMemory(&bminfoheader, sizeof(BITMAPINFOHEADER));
	bminfoheader.biSize = sizeof(BITMAPINFOHEADER);
	bminfoheader.biWidth = width;
	bminfoheader.biHeight = -height;
	bminfoheader.biPlanes = 1;
	bminfoheader.biBitCount = 32;
	bminfoheader.biCompression = BI_RGB;
	bminfoheader.biSizeImage = width * 4 * height;
	bminfoheader.biClrUsed = 0;
	bminfoheader.biClrImportant = 0;
	return bminfoheader;
}

unsigned char* pixelImage(HBITMAP& hBmp, int& width, int& height) {
	BITMAPINFOHEADER bminfoheader = getBitmapInfoHeader(width, height);
	//vector<unsigned char> pixels(width * 4 * height);
	unsigned char* pixels = new unsigned char[(width * 4 * height)];
	HDC hdc = GetDC(0);
	GetDIBits(hdc, hBmp, 0, height, &pixels[0], (BITMAPINFO*)&bminfoheader, DIB_RGB_COLORS);
	ReleaseDC(0, hdc);
	return pixels;
}

inline DWORD WINAPI tr(LPVOID) {
	RECT rect;
	using framerate = std::chrono::duration<int, std::ratio<1, 400>>;
	auto tp = std::chrono::system_clock::now() + framerate{ 1 };
	while (1) {
		if (isTrigger) {
			if (CF == GetForegroundWindow()) {
				GetClientRect(CF, &rect);
				MapWindowPoints(CF, 0, reinterpret_cast<LPPOINT>(&rect), 2);
				int width = rect.right - rect.left;
				int height = rect.bottom - rect.top;
				if (width <= 0 || height <= 0) continue;
				HDC hScreen = GetDC(CF);
				HDC hDC = CreateCompatibleDC(hScreen);
				HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, width, height);
				HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
				BitBlt(hDC, 0, 0, width, height, hScreen, 0, 0, SRCCOPY);

				int xCenter = width / 2;
				int yCenter = height / 2;

				//creating a bitmapheader for getting the dibits
				BITMAPINFOHEADER bminfoheader = getBitmapInfoHeader(width, height);

				vector<unsigned char> pPixels(width * 4 * height);
				GetDIBits(hDC, hBitmap, 0, height, &pPixels[0], (BITMAPINFO*)&bminfoheader, DIB_RGB_COLORS);

				bool found = false;

				for (int i = 29; i < 71; i++) {
					for (int j = -25; j < 25; j++) {
						int r = (int)pPixels[(width * (yCenter + i) + xCenter + j) * 4 + 2];
						int g = (int)pPixels[(width * (yCenter + i) + xCenter + j) * 4 + 1];
						int b = (int)pPixels[(width * (yCenter + i) + xCenter + j) * 4 + 0];
						if (r == 242 && g == 74 && b == 23) {
							found = true;
							break;
						}
					}

					if (found) {
						break;
					}
				}

				if (found && !(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
					if (GetAsyncKeyState(TriggerKey) & 0x8000) {
						//shoot();
						//mouse_click();
					}
				}

				// clean up
				SelectObject(hDC, old_obj);
				DeleteObject(hBitmap);
				DeleteObject(old_obj);
				DeleteDC(hDC);
				ReleaseDC(CF, hScreen);
				::ZeroMemory(&bminfoheader, sizeof(BITMAPINFOHEADER));
			}
		}
		std::this_thread::sleep_until(tp);
		tp += framerate{ 1 };
	}
}

inline DWORD WINAPI nrc(LPVOID) {
	using framerate = std::chrono::duration<int, std::ratio<1, 400>>;
	auto tp = std::chrono::system_clock::now() + framerate{ 1 };

	while (true)
	{
		if (isNoRecoil) {
			LTClientShell = read<uintptr_t>(CShell_x64 + dwLTShell);
			if (LTClientShell)
			{
				uintptr_t playerBase = read<uintptr_t>(LTClientShell + 0x90);
				if (playerBase)
				{
					float recoil = 9999999;
					float spread[4]{ 0,0,0,0 };
					float _no_spread2[5] = { 9999999, 9999999, 9999999, 9999999, 9999999 };

					writeBytes(playerBase + dwNoRecoilOffset, &spread, sizeof(spread));
					writeBytes(playerBase + dwNoRecoilOffset - 0x44, &recoil, sizeof(recoil));
					writeBytes(playerBase + dwNoRecoilOffset + 0x134, &_no_spread2, sizeof(_no_spread2));
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::seconds(2));
				}
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::seconds(2));
			}
		}
		std::this_thread::sleep_until(tp);
		tp += framerate{ 1 };
	}
	return 0;
}

inline bool World2Screen(D3DXVECTOR3* InOut);

inline void quat_ConvertFromMatrix_(float* pQuat, const float mat[4][4]);

class ModelInstance {
public:
	D3DXVECTOR3 Position();
	D3DXVECTOR4 Rotation();
};

template <typename T>
class _tools {
public:
	template <typename T>
	T GetValue(const DWORD address)
	{
		T buffer{};

		readvm(processid, (uint64_t)((uintptr_t)this + address), &buffer, sizeof(T));
		
		return buffer;
	}
};

class ObjectInstance;
class cModel : public _tools<cModel>
{
public:
	auto NumNodes(void) { return this->GetValue<int>(0x20); }
};

class ObjectInstance : public _tools< ObjectInstance>
{
public:
	ModelInstance* modelInstance();
	D3DXVECTOR3 GetBoxMin();
	D3DXVECTOR3 GetBoxMax();

	auto GetModelDB(void) { return this->GetValue<cModel*>(ModelDB); }
	auto GetCachedTransform(HMODELNODE hNode) { return this->GetValue<ObjectInstance*>(CachedTransform)->GetValue<LTMatrix>(sizeof(LTMatrix) * hNode); }

	D3DXVECTOR3 GetPosition(void) {
		return GetValue<D3DXVECTOR3>(0x19C);
	}

	inline LTRESULT GetNodeTransform(uint32 iNode, LTransform& transform, bool bInWorldSpace)
	{
		if (!this) return LT_NOTINITIALIZED;

		if (iNode > this->GetModelDB()->NumNodes())
			return LT_OUTOFMEMORY;

		LTMatrix res = GetCachedTransform(iNode);

		LTVector r0, r1, r2;
		res.GetBasisVectors(&r0, &r1, &r2);
		res.SetBasisVectors2(&r0.Normalize(), &r1.Normalize(), &r2.Normalize());

		Mat_GetTranslation(res, transform.m_Pos);
		quat_ConvertFromMatrix_((float*)&transform.m_Rot.m_Quat, res.m);
		transform.m_Scale.Init(1.0f, 1.0f, 1.0f);
		return LT_OK;
	}

	inline LTRect GetModelRect(void)
	{
		LTRect rect;

		rect.Init(0, 0, 0, 0);

		D3DXVECTOR3 min = D3DXVECTOR3(0, 0, 0);
		D3DXVECTOR3 max = D3DXVECTOR3(0, 0, 0);

		min = GetBoxMin();
		max = GetBoxMax();

		D3DXVECTOR3 points[] =
		{
			D3DXVECTOR3(min.x, min.y, min.z),
			D3DXVECTOR3(min.x, max.y, min.z),
			D3DXVECTOR3(max.x, max.y, min.z),
			D3DXVECTOR3(max.x, min.y, min.z),
			D3DXVECTOR3(max.x, max.y, max.z),
			D3DXVECTOR3(min.x, max.y, max.z),
			D3DXVECTOR3(min.x, min.y, max.z),
			D3DXVECTOR3(max.x, min.y, max.z)
		};

		D3DXVECTOR3 flb = points[3], brt = points[5], blb = points[0], frt = points[4], frb = points[2], brb = points[1], blt = points[6], flt = points[7];

		if (::World2Screen(&flb) &&
			::World2Screen(&brt) &&
			::World2Screen(&blb) &&
			::World2Screen(&frt) &&
			::World2Screen(&frb) &&
			::World2Screen(&brb) &&
			::World2Screen(&blt) &&
			::World2Screen(&flt))
		{
			D3DXVECTOR3 arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

			float left = flb.x;			// left
			float top = flb.y;			// top
			float right = flb.x;		// right
			float bottom = flb.y;		// bottom

			for (int i = 1; i < 8; i++)
			{
				if (left > arr[i].x)
					left = arr[i].x;
				if (bottom < arr[i].y)
					bottom = arr[i].y;
				if (right < arr[i].x)
					right = arr[i].x;
				if (top > arr[i].y)
					top = arr[i].y;
			}

			rect.top = top;
			rect.left = left;
			rect.right = right;
			rect.width = right - left;
			rect.height = bottom - top;
			rect.bottom = rect.top + rect.height;

			rect.centerX = rect.left + (rect.width / 2);
			rect.centerY = rect.top + (rect.height / 2);

			return rect;
		}
		return rect;
	}

	inline D3DXVECTOR3 getBonePos(int nodeID) {
		LTransform trans;
		GetNodeTransform(nodeID, trans, true);
		return trans.m_Pos;
	}
};

class CPlayer
{
public:
	ObjectInstance* hObj();
	int8_t Team();
	char* Name(int i);
	bool Has_C4();
	int8_t Health();
};

inline void quat_ConvertFromMatrix_(float* pQuat, const float mat[4][4])
{
	static int g_QNext[3] = { QY, QZ, QX };

	float diag, s;
	int i, j, k;

	diag = mat[0][0] + mat[1][1] + mat[2][2];

	if (diag < -0.999f)
	{
		i = QX;
		if (mat[QY][QY] > mat[QX][QX])
			i = QY;
		if (mat[QZ][QZ] > mat[i][i])
			i = QZ;

		j = g_QNext[i];
		k = g_QNext[j];

		s = ltsqrtf(mat[i][i] - (mat[j][j] + mat[k][k]) + 1.0f);

		pQuat[i] = s * 0.5f;
		s = 0.5f / s;
		pQuat[QW] = (mat[k][j] - mat[j][k]) * s;
		pQuat[j] = (mat[j][i] + mat[i][j]) * s;
		pQuat[k] = (mat[k][i] + mat[i][k]) * s;
		return;
	}

	s = ltsqrtf(diag + 1.0f);

	pQuat[3] = s * 0.5f;
	s = 0.5f / s;

	pQuat[0] = (mat[2][1] - mat[1][2]) * s;
	pQuat[1] = (mat[0][2] - mat[2][0]) * s;
	pQuat[2] = (mat[1][0] - mat[0][1]) * s;
}

inline int8_t CPlayer::Team() {

	if (!this)
		return NULL;

	return read<int8_t>((uintptr_t)this + 0x009);
}

//inline char* CPlayer::Name(int i)
//{
//	char pName[14]{};
//
//	read_raw((uintptr_t)this + 0x00A + i, pName, 14);
//
//	return pName;
//}

inline int8_t CPlayer::Health()
{

	if (!this)
		return NULL;

	return read<int8_t>((uintptr_t)this + 0x0044);
}

inline bool CPlayer::Has_C4()
{
	if (!this)
		return false;

	return read<bool>((uintptr_t)this + 0x0028);
}

inline ObjectInstance* CPlayer::hObj()
{
	return read<ObjectInstance*>((uintptr_t)this + 0x0000);
}

inline ModelInstance* ObjectInstance::modelInstance()
{
	return read<ModelInstance*>((uintptr_t)this + 0x0090);
}

inline D3DXVECTOR3 ModelInstance::Position()
{
	return read<D3DXVECTOR3>((uintptr_t)this + 0x19C);
}

inline D3DXVECTOR4 ModelInstance::Rotation() {
	return read<D3DXVECTOR4>((uintptr_t)this + 0x1AC);
}

inline D3DXVECTOR3 ObjectInstance::GetBoxMin()
{
	if (!this)
		return D3DXVECTOR3(0, 0, 0);

	return read<D3DXVECTOR3>((uintptr_t)this + 0x0008);
}

inline D3DXVECTOR3 ObjectInstance::GetBoxMax()
{
	if (!this)
		return D3DXVECTOR3(0, 0, 0);

	return read<D3DXVECTOR3>((uintptr_t)this + 0x0014);
}

//inline std::string GetPlayerName(CPlayer* Player)
//{
//	return Player->Name(0);
//}

inline bool World2Screen(D3DXVECTOR3* InOut) {
	D3DXVECTOR3 vScreen;
	D3DXVECTOR3 PlayerPos(InOut->x, InOut->y, InOut->z);
	D3DVIEWPORT9 viewPort = { 0 };
	D3DXMATRIX projection, view, world;
	view = read<D3DXMATRIX>(dwmatrix - 0x80);
	projection = read<D3DXMATRIX>(dwmatrix - 0x40);
	viewPort = read<D3DVIEWPORT9>(dwmatrix);

	world = D3DXMATRIX(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	D3DXVec3Project(&vScreen, &PlayerPos, &viewPort, &projection, &view, &world);

	if (vScreen.z <= 1.0f) {
		*InOut = vScreen;
		return true;
	}

	return false;
}

inline int8_t GetMyIndex()
{
	if (LTClientShell != NULL)
	{
		return read<int8_t>(LTClientShell + dwLOCAL_ENT_INDEX);
	}
}

inline void VectorAngles(D3DXVECTOR3 forward, float* angles)
{
	float yaw, pitch;

	if (forward.z == 0 && forward.x == 0)
	{
		yaw = 0;
		if (forward.z > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = atan2(-forward.y, sqrt(forward.x * forward.x + forward.z * forward.z));

		if (yaw < 0)
			yaw += 360;
		pitch = atan2(forward.x, forward.z);
		if (pitch < 0)
			pitch += 360;
	}

	angles[PITCH] = pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0.0f;
}

inline void GetAngleToTarget(D3DXVECTOR3 vTargetPos, D3DXVECTOR3& vAngles)
{
	D3DXVECTOR3 ltDelta;
	D3DXVECTOR3 vDelta;

	D3DXMATRIX View, iView;

	View = read<D3DXMATRIX>(dwmatrix - 0x80);
	D3DXMatrixInverse(&iView, NULL, &View);
	D3DXVECTOR3 i_View = D3DXVECTOR3(iView._41, iView._42, iView._43);
	D3DXVec3Subtract(&vDelta, &vTargetPos, &i_View);
	VEC_COPY(ltDelta, vDelta);
	VectorAngles(ltDelta, vAngles);

	if (vAngles.x > 180.0f)
		vAngles.x -= 360.0f;
	else if (vAngles.x < -180.0f)
		vAngles.x += 360.0f;

	if (vAngles.y > 180.0f)
		vAngles.y -= 360.0f;
	else if (vAngles.y < -180.0f)
		vAngles.y += 360.0f;
}

inline float AngleNormalize(float angle)
{
	while (angle <= -3.14f)
		angle += 6.28f;

	while (angle >= 3.14f)
		angle -= 6.28f;

	return angle;
}

inline void CalculateAngles(D3DXVECTOR3 enemy, float smooth)
{
	if (LTClientShell)
	{
		D3DXVECTOR3 newAngles(0, 0, 0);
		D3DXVECTOR3 curAngles(0, 0, 0);

		uintptr_t playerBase = read<uintptr_t>(LTClientShell + 0x90);
		if (playerBase)
		{
			auto vViewAngle = read<D3DXVECTOR3>(playerBase + ViewAngle);

			float yaw = vViewAngle.x;
			float pitch = vViewAngle.y;
			float roll = vViewAngle.z;
			//printf("Yaw: %f | Pitch: %f | Roll: %f\n", yaw, pitch, roll);

			GetAngleToTarget(enemy, newAngles);

			if (smooth > 0)
			{
				curAngles[PITCH] = AngleNormalize(pitch);
				curAngles[YAW] = AngleNormalize(yaw);

				newAngles[YAW] = AngleNormalize(curAngles[YAW] + (AngleNormalize((newAngles[YAW] - curAngles[YAW])) / smooth));
				newAngles[PITCH] = AngleNormalize(curAngles[PITCH] + (AngleNormalize((newAngles[PITCH] - curAngles[PITCH])) / smooth));

				pitch = newAngles[PITCH];
				yaw = newAngles[YAW];
			}
			else
			{
				yaw = newAngles[YAW];
				pitch = newAngles[PITCH];
			}

			roll = 0.0f;

			if (yaw > 90 || pitch < -90)
				return;

			//mouse_move(yaw, pitch);

			write<D3DXVECTOR3>(playerBase + ViewAngle, D3DXVECTOR3(yaw, pitch, roll));
		}
	}
}

inline bool IsInFOV(D3DXVECTOR3 vTargetPos, float fFov)
{
	float dx = vTargetPos.x - (WindowWidth / 2);
	float dy = vTargetPos.y - (WindowHeight / 2);

	float fov_r = fFov;

	return dx * dx + dy * dy < fov_r * fov_r;
}

inline char GetNearestDistance()
{
	char bestID = -1;
	float fDistance = INFINITE;

	int8_t MyIndex = GetMyIndex();
	CPlayer* localPlayer = GetPlayerByIndex(LTClientShell, MyIndex);

	for (int i = 0; i < 16; i++)
	{
		CPlayer* TargetPlayer = GetPlayerByIndex(LTClientShell, i);
		D3DXVECTOR3 vTargetPos = TargetPlayer->hObj()->modelInstance()->Position();

		// ignore dead players
		if (TargetPlayer->Health() <= 0)
			continue;

		//ignore teammates
		if (localPlayer->Team() == TargetPlayer->Team())
			continue;

		int fDist = VEC_DIST(localPlayer->hObj()->modelInstance()->Position(), vTargetPos) / 100;
		if (World2Screen(&vTargetPos))
		{
			if (IsInFOV(vTargetPos, fFov))
			{
				if (fDist < fDistance)
				{
					bestID = i;
					fDistance = fDist;
				}
			}
		}
	}

	return bestID;
}

inline void Aimbot_Initialize() {
	/*using framerate = std::chrono::duration<int, std::ratio<1, 400>>;
	auto tp = std::chrono::system_clock::now() + framerate{ 1 };
	while (1) {
		if (isAimbot) {*/
	if (GetAsyncKeyState(AimKey) & GetAsyncKeyState(VK_XBUTTON1))
	{
		auto bestDistanceInsideFOV = GetNearestDistance();
		if (bestDistanceInsideFOV != -1)
		{
			auto Target = GetPlayerByIndex(LTClientShell, bestDistanceInsideFOV);
			if (Target)
			{
				auto Object = Target->hObj();
				if (Object)
				{
					auto bone = Object->getBonePos(iBoneID);
					CalculateAngles(D3DXVECTOR3(bone.x, bone.y, bone.z), fSmooth);
				}
			}
		}
	}
		/*}
		std::this_thread::sleep_until(tp);
		tp += framerate{ 1 };
	}*/
}

inline DWORD WINAPI esp(LPVOID) {
	while (1) {
		//if (isEsp) {
			int8_t MyIndex = GetMyIndex();
			for (int i = 0; i < 16; i++)
			{
				CPlayer* localPlayer = GetPlayerByIndex(LTClientShell, MyIndex);
				CPlayer* TargetPlayer = GetPlayerByIndex(LTClientShell, i);
				if (TargetPlayer->Health()) {

					if (localPlayer->Team() == TargetPlayer->Team())
						continue;

					D3DXVECTOR3 vmax = TargetPlayer->hObj()->GetBoxMax();
					D3DXVECTOR3 vmin = TargetPlayer->hObj()->GetBoxMin();
					

					if ( World2Screen(&vmax) && World2Screen(&vmin))
					{
						// Calculate the player's height and width
						float box_height = vmax.y - vmin.y;
						float box_width = box_height / 1.5f; // Adjust the aspect ratio as needed

						// Center the box around the player's X-coordinate
						float box_center_x = (vmax.x + vmin.x) / 2.0f;

						// Calculate the top-left corner of the box
						float box_top_left_x = box_center_x - box_width / 2.0f;
						float box_top_left_y = vmin.y;

						//kinterface->draw_box(NULL, box_top_left_x, box_top_left_y, box_width, box_height, 2, 255, 0, 0);
					}
				}
			}
		//}
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}
}