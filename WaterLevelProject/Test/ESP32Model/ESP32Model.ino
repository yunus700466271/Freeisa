void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Memory Usage:");
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.print(" bytes");
  Serial.print("\nSketch Size: ");
  Serial.print(ESP.getSketchSize());
  Serial.print(" bytes");
  Serial.print("\nFree Sketch Space: ");
  Serial.print(ESP.getFreeSketchSpace());
  Serial.print(" bytes");
  Serial.print("\nFlash Chip Size: ");
  Serial.print(ESP.getFlashChipSize());
  Serial.print(" bytes");
}

void loop() {
  
  
  
  
  }
