#pragma once

/**
 *      Creates a progress bar which can immediatly be updated with the
 * next function 'update_progress_bar'.
 */
void create_progress_bar();

/**
 *      Updates a progress bar previously created with the function
 * 'create_progress_bar'.
 */
void update_progress_bar(unsigned long progress, unsigned long target);
