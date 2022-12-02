void read_stored_values() {
  /*
    /info:
      ISO_IDX ####
      SHOT ##
      CVAL_IDX ##
      F_LEN_IDX ##
      PRIO #
      APT_IDX ##
      SS_IDX ##
   */

  if (SD.exists("/info")) {
    Serial.println(F("/info exists, reading from it"));
    File info; 
    info = SD.open("/info", FILE_READ);
    String data = info.readString();
    Serial.println(data);

    Serial.print("pre scan: ");
    Serial.println(iso_indx);
    Serial.println(shot_number);
    Serial.println(c_indx);
    Serial.println(fl_indx);
    Serial.println(selected_prio);
    Serial.println(apt_indx);
    Serial.println(ss_indx);

    sscanf(
        data.c_str(),
        "ISO_IDX %d\nSHOT %d\nCVAL_IDX %d\nF_LEN_IDX %d\nPRIO %d\nAPT_IDX %d\nSS_IDX %d",
        iso_indx, shot_number, c_indx, fl_indx, selected_prio, apt_indx, ss_indx
      );

    Serial.print("post read: ");
    Serial.println(iso_indx);
    Serial.println(shot_number);
    Serial.println(c_indx);
    Serial.println(fl_indx);
    Serial.println(selected_prio);
    Serial.println(apt_indx);
    Serial.println(ss_indx);

    info.close();
  } else {
    update_stored_info();
  }
}

void update_stored_info() {
    Serial.println(F("[update_stored_info]: called."));
    File info = SD.open("/info", FILE_WRITE);

    info.print("ISO ");
    info.println(ISO_TABLE[iso_indx]);

    info.print("SHOT ");
    info.println(shot_number);

    info.print("CVAL_IDX ");
    info.println(c_indx);

    info.print("F_LEN_IDX ");
    info.println(fl_indx);

    info.print("PRIO ");
    info.println(selected_prio);

    info.print("APT_IDX ");
    info.println(apt_indx);

    info.print("SS_IDX ");
    info.println(ss_indx);

    info.close();
}

