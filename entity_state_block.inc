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
 * @return array
 *   Table as a render array.
 */
function _entity_state_block_get_entity_state($entity_type, $entity) {
  $entity_state_info = _entity_state_block_get_info();
  $enabled_sections = array();
  foreach ($entity_state_info[$entity_type] as $section_key => $section) {
    if ($section['enabled']) {
      $enabled_sections[] = $section_key;
    }
  }
  $data = module_invoke_all('entity_state_block_view', $entity_type, $entity, $enabled_sections);
  // Sort by weight.
  $data = array_filter(array_merge(array_flip(array_keys($entity_state_info[$entity_type])), $data), 'is_array');
  return array(
    '#theme' => 'table',
    '#rows' => $data,
  );
}

/**
 * Builds block configuration form.
 *
 * @param array $form
 *
 * @return array
 */
function _entity_state_block_configure_form($form) {
  if (!_entity_state_block_roles_assigned() && empty($_POST)) {
    drupal_set_message(t('No roles are currently assigned to the block. No one is able to see the block. Please check the "Roles" vertical tab.'), 'warning', FALSE);
  }
  $entity_state_info = _entity_state_block_get_info();
  $entity_info = entity_get_info();
  $form['main'] = array(
    '#type' => 'fieldset',
    '#title' => t('View configuration by entity type'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
  );
  $table_header = array(
    'enabled' => t('Enabled'),
    'title' => t('Title'),
    'description' => t('Description'),
    'weight' => t('Weight'),
  );
  foreach ($entity_state_info as $entity_type => $sections) {
    if (isset($entity_info[$entity_type]['label']) && !empty($sections)) {
      $weight_class = drupal_html_class('weight-' . $entity_type);
      $form['main'][$entity_type] = array(
        '#type' => 'fieldset',
        '#title' => $entity_info[$entity_type]['label'],
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
        '#tree' => TRUE,
        '#pre_render' => array('entity_state_block_configure_form_pre_render'),
        '#table_header' => $table_header,
        '#entity_type' => $entity_type,
        '#weight_class' => $weight_class,
      );
      foreach ($sections as $section_key => $section) {
        $form['main'][$entity_type][$section_key]['#entity_state'] = $section;
        $form['main'][$entity_type][$section_key]['#weight_class'] = $weight_class;
        $form['main'][$entity_type][$section_key]['weight'] = array(
          '#type' => 'textfield',
          '#attributes' => array(
            'class' => array($weight_class),
          ),
          '#default_value' => $section['weight'],
        );
        $form['main'][$entity_type][$section_key]['enabled'] = array(
          '#type' => 'checkbox',
          '#title' => 'enabled',
          '#default_value' => $section['enabled'],
        );
      }
    }
  }
  return $form;
}

/**
 * Pre-render callback for the main fieldset of the block configuration form.
 *
 * Converts contents of the child fieldsets to draggable tables.
 */
function entity_state_block_configure_form_pre_render($element) {
  $rows = array();
  foreach (element_children($element) as $key) {
    unset($element[$key]['enabled']['#title']);
    $rows[] = array(
      'data' => array(
        'enabled' => array(
          'data' => $element[$key]['enabled'],
        ),
        'title' => $element[$key]['#entity_state']['title'],
        'description' => $element[$key]['#entity_state']['description'],
        'weight' => array(
          'data' => $element[$key]['weight'],
          'class' => array($element[$key]['#weight_class']),
        ),
      ),
      'class' => array('draggable'),
    );
    unset($element[$key]);
  };
  $table_id = drupal_html_id('entity-state-block-table-' . $element['#entity_type']);
  $element['table'] = array(
    '#theme' => 'table',
    '#header' => $element['#table_header'],
    '#rows' => $rows,
    '#attributes' => array(
      'id' => $table_id,
    ),
  );
  $element['#attached']['drupal_add_tabledrag'][] = array($table_id, 'order', 'sibling', $element['#weight_class']);
  return $element;
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
  foreach ($info as $entity_type => $data) {
    foreach ($data as $section_key => $section) {
      // Fill with default values.
      $info[$entity_type][$section_key] += array(
        'weight' => 0,
        'enabled' => FALSE,
      );
      // Override with user provided values if any.
      foreach (array('weight', 'enabled') as $setting_key) {
        if (isset($settings[$entity_type][$section_key][$setting_key])) {
          $info[$entity_type][$section_key][$setting_key] = $settings[$entity_type][$section_key][$setting_key];
        }
      }
    }
  }
  drupal_alter('entity_state_block_info', $info);
  foreach ($info as $entity_type => &$data) {
    uasort($data, 'drupal_sort_weight');
  }
  unset($data);
  return $info;
}
