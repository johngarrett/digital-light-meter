void read_stored_values() {
  if (!sd_available) {
    Serial.println("SD not available, can't read stored values");
    return;
  }
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

  if (SD.exists("/info.txt")) {
    Serial.println(F("/info.txt exists, reading from it"));
    File info; 
    if (!(info = SD.open("/info.txt", FILE_READ))) {
      Serial.println(F("Unable to open /info.txt!"));
    }

    String data = info.readString();
    Serial.println(data);

    Serial.println("pre scan: ");
    Serial.println(iso_indx);
    Serial.println(shot_number);
    Serial.println(c_indx);
    Serial.println(fl_indx);
    Serial.println(selected_prio);
    Serial.println(apt_indx);
    Serial.println(ss_indx);

    int sp;
    sscanf(
        data.c_str(),
        "ISO_IDX %d\nSHOT %d\nCVAL_IDX %d\nF_LEN_IDX %d\nPRIO %d\nAPT_IDX %d\nSS_IDX %d\n",
        &iso_indx, &shot_number, &c_indx, &fl_indx, &sp, &apt_indx, &ss_indx
      );

    selected_prio = (sp == 0) ? APT_PRIO : SS_PRIO;
    Serial.println("post read: ");
    Serial.println(iso_indx);
    Serial.println(shot_number);
    Serial.println(c_indx);
    Serial.println(fl_indx);
    Serial.println(selected_prio);
    Serial.println(apt_indx);
    Serial.println(ss_indx);

    info.close();
  } else {
    Serial.println(F("/info.txt DNE, creating with update_stored_info"));
    update_stored_info();
  }
}

void update_stored_info() {
    if (!sd_available) {
      Serial.println("SD not available, can't update stored values");
      return;
    }

    Serial.println(F("[update_stored_info]: called."));
    File info = SD.open("/info.txt", O_TRUNC | FILE_WRITE);

    // we want to override the contents
    info.seek(0);

    info.print("ISO_IDX ");
    info.println(iso_indx);

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
    info.flush();
}

/**
returns:
file_name     file_size
*/
String build_file_string(File entry, boolean child) {
  String lhs = (child ? String("  ") : String("")) + String(entry.name()) + String(entry.isDirectory() ? "/" : "");
  String rhs = String(entry.size());
  String middle = "";

  // 21 chars in total, build out the middle spacing by the diff
  for (int i = 21 - lhs.length() - rhs.length(); i > 0; --i) {
    middle += String(" ");
  }

  return lhs + middle + rhs;
}
