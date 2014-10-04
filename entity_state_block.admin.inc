<?php

/**
 * Builds block configuration form.
 *
 * @param array $form
 *
 * @return array
 */
function _entity_state_block_configure_form($form) {
  module_load_include('inc', 'entity_state_block');
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
