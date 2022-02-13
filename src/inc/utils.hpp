#pragma once
using namespace std;

// using namespace rack::math;
using namespace rack;

#define NONE -1
#define LEFT -1
#define RIGHT 1
const Vec direction_vectors[4] = {Vec(0, -1), Vec(1, 0), Vec(0, 1), Vec(-1, 0)};
double sigmoid(double x);
float sigmoid(float x);
float lerpf(float a, float b, float t);
Vec lerpv(float ax, float ay, float bx, float by, float t);
Vec lerpv(Vec a, Vec b, float t);
double lerpd(double a, double b, double t);
double ilerpd(double a, double b, double p);
float ilerpf(float a, float b, float p);

double circle_dist(double a, double b);
int grid_pick(Vec pos, Vec size, Vec res);
float float_wrap(float x);
Vec equaldist(Vec s, Vec e, int i, int c);


double clamp(double x, double a, double b);
float clampf(float x, float a, float b);
float sdist(float x, float y, float _x, float _y);

double sign(double x);
int sign(float x);
int sign(int x);
double randomBi();
double cvToHertz(double cv);
int wrap(int i, int s);
bool inside(double x, double s, double e);
bool inside(int w, int h, Vec p);
double mixin(double a, double b);
float cvToBpm(float x);
void print_vec(Vec v, std::string s = "vec");
void print_rect(Rect r, std::string s = "rect");


class Midi{
  public:
    static float cvToFloatMidi(float v){
      return (clamp(v + 5.0f, 0.f, 10.58f) * 0.09451795841f) * 127.f;
    }
    static int cvToMidi(float v){
      return floor(cvToFloatMidi(v));
    }
    static float midiRoundCents(float v){
      float f = cvToFloatMidi(v);
      return f - round(f);
    }
    static int cvToMidiRound(float v){
      return round(cvToFloatMidi(v));
    }
    static int cvToOctave(float v){
      return cvToMidi(v) / 12;
    }
    static int midiToOctave(int n){
      return n / 12;
    }
    static int onOctave(int n, int d){
      int o = clamp(midiToOctave(n + d * 12), 0, 9);
      return o * 12 + n % 12;
    }
    static float midiToCv(float m){
      return (m - 60.0f) / 12.0f;
    }
    static int cvToKey(float v){
      return cvToMidi(v) % 12;
    }
    static int cvToKeyRound(float v){
      return cvToMidiRound(v) % 12;
    }
};

struct LoopClock{
  double time = 0.0;
  double pulse_width = 0.02;
  double speed = 0.25;
  void set_speed(double bpm){
    speed = bpm / 60.0;
  }
  void reset(){
    time = 0.0f;
  }
  bool step(double dt){
    dt *= speed;
    time = (time + dt) - floor(time + dt);
    return time < pulse_width;
  }

};

struct Clock{
  int state = 0;
  unsigned int cycles = 0;
  double time;//, scale, offset;
  bool running = true;
  bool loop = true;
  double offset = 0;
  bool eoc = false;
  Clock(bool r = true){
    running = r;
    loop = r;
  }
  bool update(double dt, double speed){
    if(running){
      double nt = time + dt * speed;
      time = nt - floor(nt);
      if(floor(nt) >= 1.0){
        eoc = true;
        cycles++;
        if(!loop)
          stop();
      }
      return floor(nt) > 0.0;
    }else
      return false;
  }
  void stop(){
    running = false;
    time = 0.0;
  }
  void reset(double offs = 0.0){
    eoc = false;
    running = true;
    time = 0.0;
    offset = offs;
  }
};
enum ClockState{
  STOPPED,
  RUNNING,
  EOC
};

struct TimedTrigger{
  float pulse_duration = 0.01f;
  float time = 0;
  bool up = false;
  void update(float dt){
    if(up){
      time -= dt;
      if(time < 0)
        up = false;
    }
  }
  void trigger(){
    up = true;
    time = pulse_duration;
  }
  void reset(){
    up = false;
  }
};


struct LerpClock{
  ClockState state = STOPPED;
  float time = 0.0f;
  float speed = 1.0f;
  LerpClock(float s = 1.f){
    speed = s;
  }
  float update(float dt, float mult = 1.f){
    switch(state){
      case RUNNING :
        time = time + dt * speed * mult;
        if(time >= 1.0f){
          time = 1.f;
          state = EOC;
        }
      break;

      case EOC :
        state = STOPPED;
      break;

      default : break;
    }
    return time;
  }
  void start(){
    state = RUNNING;
    time = 0.0f;
  }
};

struct ButtonTrigger{
  bool down =false;
  char state = 0;
  float threshold = 0.0f;
  bool toggle = false;
  bool on = false;
  int alt = 0;
  int alts;
  char update(float v){
     if(v > threshold){
      if(!down){
        down = true;
        on = !on;
        state = 2;
      }else
        state = 1;
     }else if(v <= threshold && down){
      down = false;
      state = -1;
     }else{
      state = 0;
     }
     return state;
  }
  char change(char dir = 1){
    alt = wrap((alt + dir), alts);
    return alt; 
  }
  ButtonTrigger(bool t = false, int as = 1){
    toggle = t;
    alts = as;
  }
};

struct Bounds{
  float start = 0.0f;
  float end = 1.0f;
  Bounds(float s, float e){
    start = s;
    end = e;
  }
};

#define RELEASED -1
#define UP 0
#define DOWN 1
#define PRESSED 2

#define RESETTING -2

struct TriggerSwitch{
  bool down =false;
  int state = UP;
  float threshold = 0.1f;
  bool toggle = false;
  bool on = false;
  int update(float v){
     if(v > threshold){
      if(!down){
        down = true;
        on = !on;
        state = PRESSED;
      }else
        state = DOWN;
     }else if(v <= threshold && down){
      down = false;
      state = RELEASED;
     }else{
      state = UP;
     }
     return state;
  }
  bool pressed(){
    return state == PRESSED;
  }
  TriggerSwitch(bool t = false){
    toggle = t;
  }
};


struct RotaryEncoder{
  float offset;
  float speed = 0.5f;
  float delta(float v){
     float r = v - offset;
     offset = v;
     return r * speed;
  }
  void init(float o){
    offset = o;
  }
};

enum StepperModes{
  STEP_RESET,
  ABS_SET,
  LEFT_RIGHT,
  X_Y
};


struct LinearStepper{
  int pos = 0;
  StepperModes mode = STEP_RESET;
  TriggerSwitch trigger_a, trigger_b;
  bool stepped = false;
  bool step(int dir, int s, int l){
    int nt = ((pos - s) + dir);
    if(nt < 0){
      pos = s + l - 1;
      return true;
    }else if(nt >= l){
      pos = s;
      return true;
    }else{
      pos = nt + s;
      return false;
    }
  }
  void step_mode(int dir = 1){
    mode = (StepperModes) ((mode + dir) % 3);
  }
  bool update(float a, float b, int s = 0, int l = 16, bool a_active = true, bool b_active = true ){
    if(stepped)
      stepped = false;
    switch(mode){
      case STEP_RESET :
      trigger_a.update(a);
      if(trigger_b.update(b) == PRESSED){
        pos = s; stepped = true;
      }else if(trigger_a.state == PRESSED){
        step(1, s, l); stepped = true;
      }break;

      case ABS_SET :{
        if(a_active){
          int nt = clamp((int)floor((rescale(a, 0.0f, 10.0f, 0, l - 1))) + s, 0, 15);
          if(nt != pos){
            if(b_active){
              if(trigger_b.update(b) > 0){
                pos = nt;
                stepped = true;
              }
            }else{
              pos = nt;
              stepped = true;;
            }
          }
        }
      }break;

      case LEFT_RIGHT :
      if(trigger_a.update(a) == PRESSED){
        step(-1, s, l); stepped = true;;
      }else if(trigger_b.update(b) == PRESSED){
        step(1, s, l); stepped = true;;
      }break;


      case X_Y : //TODO
      pos = clamp((int)floor((rescale(a, 0.0f, 10.0f, 0, l - 1))) + s, 0, 15);
      break;
    }
    return stepped;
  }
};

struct BoolSetting : MenuItem {
  bool *setting = NULL;
  void onAction(const rack::event::Action &e) override {
    *setting = !*setting;
  }
  void step() override {
    rightText = *setting ? "✔" : "";
    MenuItem::step();
  }
};

struct DivMultSlider{
  static float get(float x, float m, bool fix){
    if(x == 0)
      x = 0.0000001f;
    if(fix){
      if(x < 0.5f)
        return 1.0f / floor(rescale(x, 0.0f, 0.5f, 1.0f, (1.0f / m)) * m );
      else
        return ceil(rescale(x, 0.5f, 1.0f, 0.0f, 1.0f) * m );
    }else{
      if(x < 0.5f)
        return 1.0f / (rescale(x, 0.0f, 0.5f, (1.0f / m), 1.0f) * m);
      else
        return rescale(x, 0.5f, 1.0f, (1.0f / m), 1.0f) * m;
    }
  }
};

struct FixedParamSwitch{
  float last_value = 0.0f;
  bool changed_by(engine::Param *param){
    float v = param->getValue();
    if(v != last_value){
      last_value = v;
      return true;
    }else{
      return false;
    }
  }
};


template<typename T>
struct EnumValueItem : MenuItem {
  T *current_value;
  T value;
  void onAction(const event::Action& e) override {
    *current_value = value;
  }
};

template<typename T>
struct EnumMenuItem : MenuItem {
  T *current_value;
  std::vector<std::string> *names;
  EnumMenuItem(){
    text = "update cycle";
    rightText = RIGHT_ARROW;
  }

  Menu* createChildMenu() override {
    Menu* menu = new Menu;
    int ns = names->size();
    for (int i = 0; i < ns; i++) {
      EnumValueItem<T>* item = new EnumValueItem<T>;
      item->text = names->at(i);
      item->rightText = CHECKMARK((int)*current_value == i);
      item->value = (T) i;
      item->current_value = current_value;
      menu->addChild(item);
    }
    return menu;
  }
};


struct SampleDivider{
  int value, default_value;
  int counter = 0;
  SampleDivider(int v = 64){
    default_value = v;
    value = v;
  }
  void reset(){
    counter = 0;
    value = default_value;
  }
  bool step(){
    // if(((counter >> value) & 1) == 1){
    if((counter % value) == 0){
      counter = 0;
      counter++;
      return true; 
    }else{
      counter++;
      return false;
    }
  }
  json_t *save() {
    json_t *rootJ = json_object();
    json_object_set(rootJ, "value", json_integer(value));
    return rootJ;
  }

  void load(json_t *rootJ) {
    if(rootJ)
      value = json_integer_value(json_object_get(rootJ, "value"));
  }
};

struct SampleDividerValueItem : MenuItem {
  SampleDivider *divider;
  int value;
  void onAction(const event::Action& e) override {
    divider->value = value;
  }
};

struct SampleDividerMenuItem : MenuItem {
  SampleDivider *divider;
  SampleDividerMenuItem(){
    text = "update cycle";
    rightText = RIGHT_ARROW;
  }

  Menu* createChildMenu() override {
    Menu* menu = new Menu;
    menu->addChild(rack::construct<MenuLabel>(&MenuLabel::text, "every x samples"));
    
    for (int i = 0; i <= 10; i++) {
      int v = (int)pow(2, i);
      SampleDividerValueItem* item = new SampleDividerValueItem;
      item->text = std::to_string(v);
      item->rightText = CHECKMARK(divider->value == v);
      item->value = v;
      item->divider = divider;
      menu->addChild(item);
    }
    return menu;
  }
};

struct PatchPath : MenuItem {
  std::string path;
  void onAction(const event::Action& e) override;
};


struct PatchMenu : MenuItem {
  std::vector<std::string> patches; 
  std::string slug;
  Plugin *plugin;
  PatchMenu(){
    text = "example patches";
    rightText = RIGHT_ARROW;
  }
  Menu* createChildMenu() override {
    Menu* menu = new Menu;
    if (!patches.empty()) {
      menu->addChild(new MenuEntry);
      for (const std::string& name : patches) {
        PatchPath* patchItem = new PatchPath();
        patchItem->text = name;
        patchItem->path = asset::plugin(plugin, "patches/" + slug + "_" + name + ".vcv");
        menu->addChild(patchItem);
      }
      menu->addChild(new MenuEntry);
    }

    return menu;
  }
};


struct FloatQuantity : Quantity {
  bool *dirty = NULL;
  float *value_to_set = NULL;
  float min = 0.0f;
  float max = RACK_GRID_WIDTH;
  std::string name = "float";
  FloatQuantity(std::string n, float *v, float _min, float _max, bool *d){
    value_to_set = v;
    min = _min;
    max = _max;
    dirty = d;
    name = n;
  }
  float getMaxValue() override{
    return max;
  }
  float getMinValue() override{
    return min;
  }

  void setValue(float value) override {
    *value_to_set = math::clamp(value, getMinValue(), getMaxValue());
    *dirty = true;
  }
  float getValue() override {
    return *value_to_set;
  }

  float getDefaultValue() override {
    return 0.0;
  }
  float getDisplayValue() override {
    return getValue();
  }
  void setDisplayValue(float displayValue) override {
    // setValue(displayValue / 100);
  }
  std::string getLabel() override {
    return name;
  }
  std::string getUnit() override {
    return "";
  }
};
struct FloatSlider : ui::Slider {
  FloatSlider(std::string n, float *v, float _min, float _max, bool *d) {
    quantity = new FloatQuantity(n, v, _min, _max, d);
    box.size.x = 200.0f;
  }
  void onDragEnd(const event::DragEnd & e)override{
    ui::Slider::onDragEnd(e);
    // *dirty = true;
  }
  ~FloatSlider() {
    delete quantity;
  }
};


template <typename TModule>
void add_color_slider(Menu *menu, TModule *module){
  menu->addChild(construct<MenuLabel>());
  menu->addChild(construct<MenuLabel>(&MenuLabel::text, "module color"));
  menu->addChild(new FloatSlider("", &module->color, 0.0f, 1.0f, &module->dirty));
}