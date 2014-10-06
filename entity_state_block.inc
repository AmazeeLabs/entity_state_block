<?php

/**
 * Detects a currently viewed entity.
 *
 * @return array
 *   The first key is entity type, the second is entity ID.
 */
function _entity_state_block_get_currently_viewed_entity() {
  $null = array(NULL, NULL);
  $menu = menu_get_item();
  $current_router_path = $menu['path'];
  if ($menu['type'] == MENU_DEFAULT_LOCAL_TASK) {
    $current_router_path = $menu['tab_root'];
  }
  $view_paths = _entity_state_block_get_entity_view_paths();
  $entity_type = array_search($current_router_path, $view_paths);
  if ($entity_type === FALSE) {
    return $null;
  }
  // Find the entity in the menu map, and check if it's correct.
  $entity = FALSE;
  $id = FALSE;
  if (!empty($menu['map'])) {
    foreach ($menu['map'] as $value) {
      if (is_object($value)) {
        try {
          list($id, , ) = entity_extract_ids($entity_type, $value);
          $entity = $value;
        }
        catch (Exception $e) {
          $id = FALSE;
        }
        if ($id) {
          break;
        }
      }
    }
  }
  if (!$id) {
    return $null;
  }
  return array($entity_type, $entity);
}

/**
 * Returns table with entity state information.
 *
 * @param string $entity_type
 * @param object $entity
 *
 * @return array|null
 *   Table as a render array.
 */
function _entity_state_block_get_entity_state($entity_type, $entity) {
  $entity_state_info = _entity_state_block_get_info();
  $enabled_sections = array();
  foreach ($entity_state_info as $section_key => $section) {
    if ($section['enabled']) {
      $enabled_sections[] = $section_key;
    }
  }
  $data = module_invoke_all('entity_state_block_view', $entity_type, $entity, $enabled_sections);
  // Sort by weight.
  $data = array_filter(array_merge(array_flip(array_keys($entity_state_info)), $data), 'is_array');
  if (empty($data)) {
    return NULL;
  }
  return array(
    '#theme' => 'table',
    '#rows' => $data,
    '#attributes' => array(
      'class' => array('entity-state-block'),
    ),
  );
}

/**
 * Checks if at least one role is configured for the block.
 *
 * @return bool
 */
function _entity_state_block_roles_assigned() {
  return (bool) db_select('block_role', 'br')
    ->condition('br.module', 'entity_state_block')
    ->condition('br.delta', 'entity_state_block')
    ->countQuery()
    ->execute()
    ->fetchField();
}

/**
 * Detects entity view paths.
 *
 * @return array
 *   The keys are entity types, the values are paths.
 */
function _entity_state_block_get_entity_view_paths() {
  $paths = array();
  foreach (entity_get_info() as $entity_type => $info) {
    // First try to reuse entity_translation info, if it exists.
    if (isset($info['translation']['entity_translation']['path schemes']['default']['view path'])) {
      $path = $info['translation']['entity_translation']['path schemes']['default']['view path'];
    }
    // Then try to use the path provided by the metatag module, if any.
    elseif (isset($info['path'])) {
      $path = $info['path'];
    }
    // If nothing found: guess the view path.
    elseif ($entity_type == 'taxonomy_term') {
      $path = 'taxonomy/term/%taxonomy_term';
    }
    else {
      $path = "$entity_type/%$entity_type";
    }
    // Convert path definition to the real path used in router.
    $paths[$entity_type] = preg_replace('|%[^/]+|', '%', $path);
  }
  return $paths;
}

/**
 * Collects information from hook_entity_state_block_info implementations.
 */
function _entity_state_block_get_info() {
  module_load_include('contrib.inc', 'entity_state_block');
  $settings = variable_get('entity_state_block_settings', array());
  $info = module_invoke_all('entity_state_block_info');
  foreach ($info as $section_key => $section) {
    // Fill with default values.
    $info[$section_key] += array(
      'weight' => 0,
      'enabled' => FALSE,
    );
    // Override with user provided values if any.
    foreach (array('weight', 'enabled') as $setting_key) {
      if (isset($settings[$section_key][$setting_key])) {
        $info[$section_key][$setting_key] = $settings[$section_key][$setting_key];
      }
    }
  }
  drupal_alter('entity_state_block_info', $info);
  uasort($info, 'drupal_sort_weight');
  return $info;
}
