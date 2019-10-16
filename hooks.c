int next_sfc_flag = 0;
void next_sfc(){
	next_sfc_flag = 1;
}

Layout* old_layout = NULL;
void all_windows_hook(Client* c, int time){
	if(next_sfc_flag){
		if(time == 0){
			Layout *cl;
			for(cl = (Layout *)layouts; cl != selmon->lt[selmon->sellt]; cl++);
            Arg a = {.v = NULL};
			setlayout(&a);
		} else {
			sfc(c, 0.85);
			next_sfc_flag = 0;
			//TODO: set previous
		}
	}
}
